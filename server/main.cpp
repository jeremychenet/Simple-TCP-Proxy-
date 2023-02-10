#include <iostream>
#include <boost/asio.hpp>
#include <array>

using namespace std;
using boost::asio::ip::tcp;

class TCPServer
{
public:
    TCPServer(boost::asio::io_context& io_context, int port, bool secret_mode)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), _secret_mode(secret_mode)
    {
        start_accept();
    }

    ~TCPServer()
    {
        for (auto socket : _sockets)
            delete std::get<SOCKET>(socket);
    }


private:

    void send_to_all_clients(const std::string& message)
    {
        for (auto socket : _sockets) {
            boost::asio::async_write(*std::get<SOCKET>(socket), boost::asio::buffer(message), [this](boost::system::error_code ec, std::size_t length)
            {
                if (ec) {
                    std::cout << "Error: " << ec.message() << std::endl;
                }
            });
        }
    }
    void start_accept()
    {
        // Create a new potential socket:
        tcp::socket *socket = new tcp::socket(acceptor_.get_executor());
        acceptor_.async_accept(*socket, [this, socket](boost::system::error_code ec)
        {
            if (!ec) {
                std::tuple<tcp::socket*, std::string> new_client = std::make_tuple(socket, "");
                std::cout << "New client connected." << std::endl;
                _sockets.push_back(new_client);
                _nb_clients += 1;
                start_read(*socket);
            }
            else {
                delete socket;
            }
            // Get ready for a new client connection until we have 2 clients:
            if (_nb_clients < 2) {
                send_to_all_clients("OK 1");;
            }
            else {
                std::cout << "Both clients are connected." << std::endl;
                send_to_all_clients("OK 2");
            }
            start_accept();
        });
    }

    void start_read(tcp::socket& socket)
    {
        socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH), [this, &socket](boost::system::error_code ec, std::size_t length)
        {
            if (!ec) {
                if (strncmp(_data, "[CMD]SETSECRET ", LENGTH_SECRET_CMD) == 0) {
                    std:string secret = std::string(_data).substr(std::string("[CMD]SETSECRET ").length());
                    set_secret_of_the_client(socket, secret);
                    find_client_with_same_secret(socket, secret);
                } else {
                    std::cout << "Data received: " << std::string(_data, length) << std::endl;
                    send_message_to_the_other_client(socket, std::string(_data, length));
                    memset(_data, 0, MAX_LENGTH);
                }
                start_read(socket);
            } else {
                if (ec.message() == "End of file")
                    std::cout << "Client disconnected." << std::endl;
                else
                    std::cout << "Error: " << ec.message() << std::endl;
                for (auto it = _sockets.begin(); it != _sockets.end(); ++it) {
                    if (std::get<SOCKET>(*it) == &socket) {
                        _sockets.erase(it);
                        break;
                    }
                }
                _nb_clients -= 1;
            }
        });
    }

    void send_message_to_the_other_client(tcp::socket& socket, const std::string& message)
    {
        int i = 0;
        for (auto s : _sockets) {
            if (std::get<SOCKET>(s) != &socket) {
                boost::asio::async_write(*std::get<SOCKET>(s), boost::asio::buffer(message), [this](boost::system::error_code ec, std::size_t length)
                {
                    if (ec) {
                        std::cout << "Error: " << ec.message() << std::endl;
                    }
                });
            }
        }
    }

    void send_message_to_a_specific_client(tcp::socket& socket, const std::string& message)
    {
        boost::asio::async_write(socket, boost::asio::buffer(message), [this](boost::system::error_code ec, std::size_t length)
        {
            if (ec) {
                std::cout << "Error: " << ec.message() << std::endl;
            }
        });
    }

    void set_secret_of_the_client(tcp::socket& socket, const std::string& secret)
    {
        if (!_secret_mode)
            return;
        for (auto it = _sockets.begin(); it != _sockets.end(); ++it) {
            if (std::get<SOCKET>(*it) == &socket) {
                std::get<SECRET>(*it) = secret;
                break;
            }
        }
    }

    void find_client_with_same_secret(tcp::socket& socket, const std::string& secret)
    {
        for (auto s : _sockets) {
            if (std::get<SOCKET>(s) != &socket && std::get<SECRET>(s) == secret) {
                    send_message_to_a_specific_client(socket, "OK SECRET");
                    send_message_to_a_specific_client(*std::get<SOCKET>(s), "OK SECRET");
                }
            }
    }


    tcp::acceptor acceptor_;
    std::vector<std::tuple<tcp::socket*, std::string>> _sockets;

    bool _secret_mode = false;
    enum { 
        SOCKET = 0,
        SECRET = 1,
        LENGTH_SECRET_CMD = 15,
        MAX_LENGTH = 1024,
    };
    char _data[MAX_LENGTH] = {0};
    short _nb_clients = 0;
};

void run_server_session(int port, bool secret_mode = false)
{
    boost::asio::io_context io_context;
    TCPServer server(io_context, port, secret_mode);
    std::cout << "Server started, waiting for connection..." << std::endl;
    io_context.run();
}

int main(int ac, char* av[])
{
    try {
        if ((ac == 2 && std::string(av[1]) == "-h") || (ac < 2 || ac > 3)) {
            cerr << "Usage: TCPServer <port> -secret" << endl;
            return 0;
        }
        bool secret_mode = false;
        if (ac == 3 && av[2] == std::string("--secret"))
            secret_mode = true;
        run_server_session(atoi(av[1]), secret_mode);
    } catch (exception& e) {
        cerr << "Exception: " << e.what() << endl;
    }
    return 0;
}
