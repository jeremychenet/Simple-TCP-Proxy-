#include <iostream>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    typedef std::shared_ptr<TcpConnection> pointer;

    static pointer create(boost::asio::io_context& io_context)
    {
        return pointer(new TcpConnection(io_context));
    }

    tcp::socket& socket()
    {
        return socket_;
    }

    void start()
    {
        boost::asio::async_read(socket_, boost::asio::buffer(data_),
            [this, self = shared_from_this()](boost::system::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    std::cout << "Received message: ";
                    std::cout.write(data_.data(), length);
                    std::cout << std::endl;
                }
                else
                {
                    std::cerr << "Error: " << ec.message() << std::endl;
                }
            });
    }

private:
    TcpConnection(boost::asio::io_context& io_context)
        : socket_(io_context)
    {
    }

    tcp::socket socket_;
    std::array<char, 1024> data_;
};

class TcpServer
{
public:
    TcpServer(boost::asio::io_context& io_context)
        : acceptor_(io_context, tcp::endpoint(tcp::v4(), 12345))
    {
        startAccept();
    }

private:
    void startAccept() 
    {
        TcpConnection::pointer newConnection = TcpConnection::create(acceptor_.get_executor().get_context());
        acceptor_.async_accept(newConnection->socket(),
            [this, newConnection](boost::system::error_code ec)
            {
                if (!ec)
                {
                    newConnection->start();
                    std::cout << "Accepted connection from " << newConnection->socket().remote_endpoint().address().to_string() << std::endl;
                }
                else
                {
                    std::cerr << "Error: " << ec.message() << std::endl;
                }
                startAccept();
            });
    }

    tcp::acceptor acceptor_;
};

int main()
{
    try
    {
        boost::asio::io_context io_context;
        auto& executor = io_context.get_executor().context();
        TcpServer server(io_context);
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
