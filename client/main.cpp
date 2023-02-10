#include "client.hpp"

void random_sleep() {
  std::srand(std::time(0));
  int random_sleep_time = (std::rand() % (300000 - 500000 + 1)) + 500000;
  std::this_thread::sleep_for(std::chrono::microseconds(random_sleep_time));
}

void run_tests(int ac, char **av)
{
    std::cout << "Tests for client started." << std::endl;
    std::string secret = std::string(av[4]);
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::resolver resolver(io_context);
    TcpClient client(io_context, true, secret);
    client.start(resolver.resolve(av[2], av[3]));
    std::thread t([&io_context](){ io_context.run(); });
    random_sleep();
    client.sendMessage("TESTING SERVER");
    random_sleep();
    client.sendMessage("[CMD]ECHOREPLY echoreply_test");
    usleep(500000);
    std::cout << "Tests for client ended." << std::endl;
    if (t.joinable())
        t.join();
    exit(0);
}

void run_client_session(int ac, char **av)
{
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::resolver resolver(io_context);
    std::string secret = "";
    std::string message;
    bool secret_mode = false;
    if (ac == 5) {
        secret_mode = true;
        secret = std::string(av[4]);
    }

    // Create the client.
    TcpClient client(io_context, secret_mode, secret);

    // Start the client session.
    client.start(resolver.resolve(av[1], av[2]));

    //thread to run the io_context.
    std::thread t([&io_context](){ io_context.run(); });

    // Starting prompt here.
    usleep(500000);
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
        if (ac == 2 && av[1] == std::string("-h") || (ac < 3 || ac > 5)) {
            std::cerr << "Usage: ./client_side [host] [port] [OPTION]" << std::endl;
            std::cerr << "Options:" << std::endl;
            std::cerr << "\t--proxy " << std::endl;
            std::cerr << "\t--secret [secret]\t\tsecret for the client" << std::endl;
            std::cerr << "\t-h\t\t\tDisplay this help" << std::endl;
            return 0;
        }
        if (ac == 5 && av[1] == std::string("--test"))
            run_tests(ac, av);
        run_client_session(ac, av);

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 84;
    }
    return 0;
}