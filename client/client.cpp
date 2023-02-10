#include "client.hpp"

void TcpClient::start(tcp::resolver::results_type endpoints)
{
    _endpoints = endpoints;
    start_connect(endpoints.begin());
}

void TcpClient::start_connect(tcp::resolver::results_type::iterator endpoint_iter)
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

void TcpClient::handle_connect(const boost::system::error_code& ec,
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

void TcpClient::sendMessage(const std::string& message)
{
    boost::asio::async_write(_socket, boost::asio::buffer(message), [this](boost::system::error_code ec, std::size_t length)
    {
        if (ec) {
            std::cout << "Error: " << ec.message() << std::endl;
        }
    });
}

void TcpClient::start_read(void)
{
    _socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH),
                            boost::bind(&TcpClient::handle_read, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

void TcpClient::handle_command(std::string message)
{
    if (message.find("OK 2") == 0) {
        return;
    }
    if (message.find("[CMD]ECHOREPLY ") == 0) {
        std::cout << "Client Received: " << message << std::endl;
        sendMessage(message.substr(std::string("[CMD]ECHOREPLY ").length()));
    } else {
        std::cout << "Client Received: " << message << std::endl;
    }
}

void TcpClient::handle_read(const boost::system::error_code& ec, std::size_t n)
{
    if (!ec) {
        if (_other_client_connected) {
            handle_command(std::string(_data));
        }
        if (!_other_client_connected && std::string(_data).find("OK 1") != std::string::npos) {
            std::cout << "Waiting for the other client to connect..." << std::endl;
        }
        if (!_secret_mode && std::string(_data).find("OK 2") != std::string::npos) {
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

void TcpClient::handle_timer(const boost::system::error_code &error)
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