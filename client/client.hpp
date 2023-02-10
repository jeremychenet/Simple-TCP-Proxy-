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

    void start(tcp::resolver::results_type endpoints);
    void start_connect(tcp::resolver::results_type::iterator endpoint_iter);
    void handle_connect(const boost::system::error_code& ec,
                        tcp::resolver::results_type::iterator endpoint_iter);
    void sendMessage(const std::string& message);

private:
    void start_read(void);
    void handle_command(std::string message);
    void handle_read(const boost::system::error_code& ec, std::size_t n);
    void handle_timer(const boost::system::error_code &error);

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
