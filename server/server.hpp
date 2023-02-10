#include <iostream>
#include <boost/asio.hpp>
#include <array>

using namespace std;
using boost::asio::ip::tcp;

class TCPServer
{
public:
    TCPServer(boost::asio::io_context& io_context, int port, bool secret_mode, bool test_mode)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), _secret_mode(secret_mode), _test_mode(test_mode)
    {
        start_accept();
    }

    ~TCPServer()
    {
        for (auto socket : _sockets)
            delete std::get<SOCKET>(socket);
    }

private:
    void send_to_all_clients(const std::string& message);
    void start_accept(void);
    void start_read(tcp::socket& socket);
    void send_message_to_the_other_client(tcp::socket& socket, const std::string& message);
    void send_message_to_a_specific_client(tcp::socket& socket, const std::string& message);
    void set_secret_of_the_client(tcp::socket& socket, const std::string& secret);
    void find_client_with_same_secret(tcp::socket& socket, const std::string& secret);

    tcp::acceptor acceptor_;
    std::vector<std::tuple<tcp::socket*, std::string>> _sockets;

    enum { 
        SOCKET = 0,
        SECRET = 1,
        LENGTH_SECRET_CMD = 15,
        MAX_LENGTH = 1024,
    };
    bool _secret_mode = false;
    bool _test_mode = false;
    char _data[MAX_LENGTH] = {0};
    short _nb_clients = 0;
};