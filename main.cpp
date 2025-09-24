#include <iostream>
#include <sstream>


int main()
{
    std::string input;

    while (true)
    {
        std::getline(std::cin, input);
        std::stringstream ss{input};
        std::string command;
        std::getline(ss, command);

        if (command == "CREATE")
        {

        }
        else if (command == "DELETE")
        {

        }
        else if (command == "MODIFY")
        {

        }   
        else if (command == "GET")
        {

        }
        else if (command == "QUIT")
        {
            break;
        }   
    }

    return 0;
}