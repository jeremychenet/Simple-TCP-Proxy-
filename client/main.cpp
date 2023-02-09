#include <iostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class TcpClient
{
public:
    TcpClient(boost::asio::io_service& io_service)
        : socket_(io_service)
    {
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 12345);
        socket_.connect(endpoint);
        std::cout << "Connected to server" << std::endl;
    }

    void sendMessage(const std::string& message)
    {
        boost::asio::write(socket_, boost::asio::buffer(message));
        std::cout << "Message sent: " << message << std::endl;
    }

private:
    tcp::socket socket_;
};

int main()
{
    try
    {
        boost::asio::io_service io_service;
        TcpClient client(io_service);

        std::string message;
        while (true)
        {
            std::cout << "> ";
            std::getline(std::cin, message);
            if (message == "exit")
                break;
            client.sendMessage(message);
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}

