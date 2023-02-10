#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <thread>
#include <chrono>

using boost::asio::ip::tcp;

class TcpClient
{
public:
    TcpClient(boost::asio::io_context& io_context, bool secret_mode, std::string secret) : _socket(io_context), _timer(io_context), _secret_mode(secret_mode), _secret(secret)
    {}

    ~TcpClient() {
        _socket.close();
        std::cout << "Disconnected from server." << std::endl;
    }

    void start(tcp::resolver::results_type endpoints)
    {
        _endpoints = endpoints;
        start_connect(endpoints.begin());
    }
    void start_connect(tcp::resolver::results_type::iterator endpoint_iter)
    {
        if (endpoint_iter != _endpoints.end())
        {
            std::cout << "Trying to connect to " << endpoint_iter->endpoint() << "...\n";

            // Start the asynchronous connect operation.
            _socket.async_connect(endpoint_iter->endpoint(),
                                  boost::bind(&TcpClient::handle_connect, this,
                                              boost::placeholders::_1, endpoint_iter));
        }
    }

    void handle_connect(const boost::system::error_code& ec,
                        tcp::resolver::results_type::iterator endpoint_iter)
    {
        if (ec) {
            std::cout << "Connect error: " << ec.message() << "\n";
            _socket.close();
            start_connect(++endpoint_iter);
        } else {
            std::cout << "Connected to " << endpoint_iter->endpoint() << "\n";
            start_read();
            if (_secret_mode) {
                sendMessage("[CMD]SETSECRET " + _secret);
            }
            _timer.expires_from_now(std::chrono::seconds(7));
            _timer.async_wait(boost::bind(&TcpClient::handle_timer, this, _1));
        }
    }

    void sendMessage(const std::string& message)
    {
        boost::asio::async_write(_socket, boost::asio::buffer(message), [this](boost::system::error_code ec, std::size_t length)
        {
            if (ec) {
                std::cout << "Error: " << ec.message() << std::endl;
            }
        });
    }

private:
    void start_read()
    {
        _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                                boost::bind(&TcpClient::handle_read, this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
    }

    void handle_command(std::string message) {

        // if message starts with "[CMD]ECHOREPLY "
        if (message.find("[CMD]ECHOREPLY ") == 0) {
            std::cout << "Received: " << message << std::endl;
            sendMessage(message.substr(std::string("[CMD]ECHOREPLY ").length()));
        } else {
            std::cout << "Received: " << message << std::endl;
        }
    }

    void handle_read(const boost::system::error_code& ec, std::size_t n)
    {
        if (!ec) {
            if (_other_client_connected) {
                handle_command(std::string(_data));
            }
            if (!_other_client_connected && std::string(_data).find("OK 1") != std::string::npos) {
                std::cout << "Waiting for the other client to connect..." << std::endl;
            }
            if (!_other_client_connected && !_secret_mode && std::string(_data).find("OK 2") != std::string::npos) {
                _timer.cancel();
                std::cout << "Other client is now connected." << std::endl;
                _other_client_connected = true;
            }
            if (!_other_client_connected && _secret_mode && std::string(_data).find("OK SECRET") != std::string::npos) {
                _timer.cancel();
                std::cout << "Other client with the same secret is now connected." << std::endl;
                _other_client_connected = true;
            }
            memset(_data, 0, MAX_LENGTH);
            start_read();
        } else {
            std::cout << "Server disconnected." << std::endl;
        }
    }

    void handle_timer(const boost::system::error_code &error)
    {
        if (!error)
        {
            if(!_secret_mode)
                std::cout << "No other client has connected after 7 seconds. Closing the program." << std::endl;
            else
                std::cout << "No other client with the same password has connected after 7 seconds. Closing the program." << std::endl;
            _socket.close();
            exit(84);
        }
    }

    tcp::socket _socket;
    std::thread _reading_thread;
    tcp::resolver::results_type _endpoints;
    std::string _input_buffer;
    boost::asio::steady_timer _timer;
    std::string _secret = "";

    enum { MAX_LENGTH = 1024 };
    char _data[MAX_LENGTH] = {0};
    bool _other_client_connected = false;
    bool _secret_mode = false;
};

void run_client_session(int ac, char **av)
{
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::resolver resolver(io_context);
    bool secret_mode = false;
    if (ac == 4)
        secret_mode = true;
    std::string secret = "";
    if (secret_mode)
        secret = std::string(av[3]);
    TcpClient client(io_context, secret_mode, secret);

    // Start the client session.
    client.start(resolver.resolve(av[1], av[2]));

    //thread to run the io_context.
    std::thread t([&io_context](){ io_context.run(); });

    // Starting prompt here.
    usleep(100000);
    std::string message;
    while (true)
    {
        std::cout << "> ";
        std::getline(std::cin, message);
        if (message == "exit")
            break;
        client.sendMessage(message);
    }
    if (t.joinable())
        t.join();
}

int main(int ac, char **av)
{
    try {
        if (ac == 2 && av[1] == std::string("-h")) {
            std::cout << "Usage: ./client [host] [port] [secret]" << std::endl;
            return 0;
        }
        run_client_session(ac, av);

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}