#include "server.hpp"

const int TEST_DELAY = 5;

void run_server_session(int port, bool secret_mode = false, bool test_mode = false)
{
    boost::asio::io_context io_context;
    TCPServer server(io_context, port, secret_mode, test_mode);
    if (secret_mode)
        std::cout << "Server started in secret mode, waiting for connection..." << std::endl;
    else
        std::cout << "Server started, waiting for connection..." << std::endl;
    if (test_mode) {
        std::cout << "Tests for server started." << std::endl;
        //Start a timer, run io_service.run and quit after 3 seconds.
        boost::asio::deadline_timer t(io_context, boost::posix_time::seconds(TEST_DELAY));
        t.async_wait([&](const boost::system::error_code&){ io_context.stop(); });
        io_context.run();
        std::cout << "Tests for server ended." << std::endl;
    } else {
        io_context.run();
    }
}

int main(int ac, char* av[])
{
    try {
        if ((ac == 2 && std::string(av[1]) == "-h") || (ac < 2 || ac > 3)) {
            std::cerr << "Usage: ./server_side [OPTION]" << std::endl;
            std::cerr << "Options:" << std::endl;
            std::cerr << "\t--proxy [port]\t\tPort to listen on" << std::endl;
            std::cerr << "\t--secret [port]\t\tPort to listen on with secret mode" << std::endl;
            std::cerr << "\t-h\t\t\tDisplay this help" << std::endl;
            return 0;
        }
        bool secret_mode = false;
        if (ac == 3 && av[1] == std::string("--secret"))
            secret_mode = true;
        if (ac == 3 && av[1] == std::string("--test"))
            run_server_session(atoi(av[2]), true, true);
        else
            run_server_session(atoi(av[2]), secret_mode, false);
    } catch (exception& e) {
        cerr << "Exception: " << e.what() << endl;
        return 84;
    }
    return 0;
}
