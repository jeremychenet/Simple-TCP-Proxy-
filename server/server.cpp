#include "server.hpp"

void TCPServer::send_to_all_clients(const std::string& message)
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

void TCPServer::start_accept(void)
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

void TCPServer::start_read(tcp::socket& socket)
{
    socket.async_read_some(boost::asio::buffer(_data, MAX_LENGTH), [this, &socket](boost::system::error_code ec, std::size_t length)
    {
        if (!ec) {
            if (strncmp(_data, "[CMD]SETSECRET ", LENGTH_SECRET_CMD) == 0) {
                std:string secret = std::string(_data).substr(std::string("[CMD]SETSECRET ").length());
                set_secret_of_the_client(socket, secret);
                find_client_with_same_secret(socket, secret);
            } else {
                std::cout << "Server Received: " << std::string(_data, length) << std::endl;
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

void TCPServer::send_message_to_the_other_client(tcp::socket& socket, const std::string& message)
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

void TCPServer::send_message_to_a_specific_client(tcp::socket& socket, const std::string& message)
{
    boost::asio::async_write(socket, boost::asio::buffer(message), [this](boost::system::error_code ec, std::size_t length)
    {
        if (ec) {
            std::cout << "Error: " << ec.message() << std::endl;
        }
    });
}

void TCPServer::set_secret_of_the_client(tcp::socket& socket, const std::string& secret)
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

void TCPServer::find_client_with_same_secret(tcp::socket& socket, const std::string& secret)
{
    for (auto s : _sockets) {
        if (std::get<SOCKET>(s) != &socket && std::get<SECRET>(s) == secret) {
                send_message_to_a_specific_client(socket, "OK SECRET");
                send_message_to_a_specific_client(*std::get<SOCKET>(s), "OK SECRET");
            }
        }
}