// Order Book.

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <list>


struct Order
{
    enum class Verb
    {
        BUY,
        SELL
    };

    std::string orderID;
    std::string productID;
    Verb verb;
    uint32_t price;
    uint32_t quantity;

    std::string to_string()
    {
        std::string verb_s = (verb == Verb::BUY) ? "BUY" : "SELL";
        return orderID + " " + productID + " " + verb_s + " " + 
          std::to_string(price) + " " + std::to_string(quantity);
    }
};

class OrderBook
{
  public:
    std::string create(std::string& parameters);
    std::string del(std::string& parameters);
    std::string modify(std::string& parameters);
    std::string get(std::string& parameters);

  private:
    std::unordered_map<std::string, Order> orders;
};

std::string OrderBook::create(std::string& parameters)
{
    // CREATE OrderId ProductId Verb Price Quantity
    //  E.g.: CREATE 1 1 BUY 1 1
    std::stringstream ss{parameters};
    std::string orderID, productID, verb_s, price_s, quantity_s;
    std::getline(ss, orderID, ' ');
    if (orders.find(orderID) != orders.end())
    {
        return "ERROR";
    }
    std::getline(ss, productID, ' ');
    std::getline(ss, verb_s, ' ');
    std::getline(ss, price_s, ' ');
    std::getline(ss, quantity_s);
    std::cout << "  Spliced: " << orderID << " " << productID << " " << verb_s 
      << " " << price_s << " " << quantity_s << "\n";

    Order new_order;
    new_order.orderID = orderID;
    new_order.productID = productID;
    new_order.verb = verb_s == "BUY" ? Order::Verb::BUY : Order::Verb::SELL;
    new_order.price = stoul(price_s);
    new_order.quantity = stoul(quantity_s);

    orders[orderID] = new_order;
    std::cout << "  Created: " << new_order.to_string() << "\n";
    
    return "OK";
}

std::string OrderBook::del(std::string& parameters)
{
    std::stringstream ss{parameters};
    // No need to call clear() since it's a new stream.
    std::string orderID;
    std::getline(ss, orderID);
    
    if (orders.erase(orderID) == 0)
    {
        return "ERROR";
    }

    std::cout << "  Deleted: " << orderID << "\n";
    return "OK";
}

std::string OrderBook::modify(std::string& parameters)
{
    std::stringstream ss{parameters};
    std::string orderID, price_s, quantity_s;
    std::getline(ss, orderID, ' ');
    if (orders.find(orderID) == orders.end())
    {
        return "ERROR";
    }
    std::getline(ss, price_s, ' ');
    std::getline(ss, quantity_s);
    auto price = stoul(price_s);
    auto quantity = stoul(quantity_s);

    auto& order = orders[orderID];
    order.price = price;
    order.quantity = quantity;

    std::cout << "  Modified: " << order.to_string() << "\n";
    return "OK";
}    

std::string OrderBook::get(std::string& parameters)
{
    std::stringstream ss{parameters};
    std::string orderID;
    std::getline(ss, orderID, ' ');
    if (orders.find(orderID) == orders.end())
    {
        return "ERROR";
    }

    auto& order = orders[orderID];
    return "OK " + order.to_string();
}


int main()
{
    OrderBook order_book;
    std::unordered_map<std::string, std::list<Order&>> to_buy;
    std::unordered_map<std::string, std::list<Order&>> to_sell;

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
            
        }
        else if (command == "QUIT")
        {
            break;
        }   
    }

    return 0;
}