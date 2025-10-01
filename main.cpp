// TODO:
// - Validazione input;
// - Storico dei comandi;
// - Case insensitive;
// - authentication/authorization

// C++ standard
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <map>
// POSIX
#include <sys/socket.h> // For APIs like socket(), bind(), etc.
#include <netinet/in.h> // For the socket address struct like sockaddr_in.
#include <unistd.h> // For close(), read(), write().
// Custom
#include "order_book.hpp"
#include "order_book_parser.hpp"


void network_mod();

int main(int argc, char** argv)
{
    if (argc == 1)
    {
        network_mod();
        return 0;
    }

    OrderBookParser order_book;

    while (true)
    {
        std::cout << "Insert COMMAND: " ;
        
        std::string input;
        std::getline(std::cin, input);
        std::stringstream ss{input};
        std::string command;
        std::getline(ss, command, ' ');
        std::string parameters;
        std::getline(ss, parameters);

        if (command == "CREATE")
        {
            std::cout << "  Command: " << command << "\n";
            std::cout << "  Parameters: " << parameters << "\n";

            auto result = order_book.create(parameters);
            std::cout << "  Result of CREATE: " << result << "\n";
        }
        else if (command == "DELETE")
        {
            // DELETE OrderId
            std::cout << "  Command: " << command << "\n";
            std::cout << "  Parameters: " << parameters << "\n";

            auto result = order_book.del(parameters);
            std::cout << "  Result of DELETE: " << result << "\n";
        }
        else if (command == "MODIFY")
        {
            // MODIFY OrderId Price Quantity
            //  E.g.: MODIFY 1 2 2
            std::cout << "  Command: " << command << "\n";
            std::cout << "  Parameters: " << parameters << "\n";

            auto result = order_book.modify(parameters);
            std::cout << "  Result of MODIFY: " << result << "\n";
        }   
        else if (command == "GET")
        {
            // GET OrderId
            std::cout << "  Command: " << command << "\n";
            std::cout << "  Parameters: " << parameters << "\n";

            auto result = order_book.get(parameters);
            std::cout << "  Result of GET: " << result << "\n";
        }
        else if (command == "AGGREGATED_BEST")
        {
            // AGGREGATED_BEST ProductId
            std::cout << "  Command: " << command << "\n";
            std::cout << "  Parameters: " << parameters << "\n";

            auto result = order_book.aggregated_best(parameters);
            std::cout << "  Result of AGGREGATED_BEST: " << result << "\n";
        }
        else if (command == "QUIT")
        {
            break;
        }
    }

    return 0;
}

void network_mod(int client_fd)
{
    // Parameters:
    // 1. the domain of addresses, like IPv4, IPv6, local sockets;
    // 2. the transport layer, like TCP, UDP or raw;
    // 3. the protocol, like TCP/IP, UDP/IP or let-the-system-decide (0).
    // Paramer #3 allows custom protocol still based on known IP/TCP/UDP.
    auto socket_fd = socket(AF_INET, SOCK_STREAM, 0); // IPv4/TCP.
    // TODO: Check the File Descriptor is not -1.

    // Parameters:
    // 1. the socker File Descriptor;
    // 2. the socket address struct;
    // 3. the length of the socket address struct.
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Any address.
    address.sin_port = htons(8080); // From Machine to Network byte order.
    auto result_bind = bind(socket_fd, (struct sockaddr*) &address, sizeof(address));
    // TODO: check the bind didn't fail.

    // Parameters:
    // 1. the socket to listen on;
    // 2. the backlog i.e. queue size of pending clients (tipically 5-128).
    constexpr int backlog = 10;
    auto result_listen = listen(socket_fd, backlog);

    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(client_address);
    while (true)
    {
        auto client_fd = accept(socket_fd, (struct sockaddr*) &client_address, 
          &client_address_size);
        // TODO: Check client_fd not <0.

        // Handle client 
        //read()/recv() - Read data from client
        //write()/send() - Send data to client

        // TODO:
        // - can I make it trasparent if the I/O happens locally or via socket?

        close(client_fd);
    }

    close(socket_fd);
}