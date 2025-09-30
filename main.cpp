// Order Book: table of buy orders (bids) and sell orders (asks).
// Bid: price a buyer is willing to pay for a quantity of product; the best bid 
//  is the highest price.
// Ask: price a seller is willing to accept for a quantity of product; the best 
//  ask is the lowest price.
// Spread: best_ask - best_bid. Its value can be:
//  > 0: represents the implicit liquidity/inefficiency cost of entering or 
//       exiting the market;
//  == 0: perfectly tight spread, when the market is balanced;
//  < 0: (crossed book) cannot really exist, except for a brief moment before 
//       the matching engine executes the trade.
// The spread is one indicator of liquidity: a tight spread means low cost of 
//  trading and high competition. But spread alone is not enough. Another key 
//  factor is Depth: large quantities available at each price level mean high 
//  depth, and therefore a more liquid market.
// In terms of liquidity, an order can be:
// - liquidity provider: if it does not cross the book, it stays in the book and 
//   adds available volume, increasing liquidity;
// - liquidity taker: if it is marketable, i.e. it crosses the book, it triggers 
//   a match and consumes liquidity.

// TODO:
// - Validazione input;
// - Storico dei comandi;
// - Case insensitive.
// - authentication/authorization

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <map>

#include "order_book.hpp"


class OrderBookParser
{
  public:
    std::string create(std::string& parameters);
    std::string del(std::string& parameters);
    std::string modify(std::string& parameters);
    std::string get(std::string& parameters);
    std::string aggregated_best(std::string& parameters);

  private:
    OrderBook order_book;
};

std::string OrderBookParser::create(std::string& parameters)
{
    // CREATE OrderId ProductId Verb Price Quantity
    //  E.g.: CREATE 1 1 BUY 1 1
    std::stringstream ss{parameters};
    std::string orderID, productID, verb_s, price_s, quantity_s;
    std::getline(ss, orderID, ' ');
    std::getline(ss, productID, ' ');
    std::getline(ss, verb_s, ' ');
    std::getline(ss, price_s, ' ');
    std::getline(ss, quantity_s);

    auto verb = verb_s == "BUY" ? Order::Verb::BUY : Order::Verb::SELL;
    return order_book.create(orderID, productID, verb, std::stoul(price_s), 
      std::stoul(quantity_s)) ? "OK" : "ERROR";
}
std::string OrderBookParser::del(std::string& parameters)
{
    // DELETE OrderId
    //  E.g.: DELETE 1

    std::stringstream ss{parameters};
    // No need to call clear() since it's a new stream.
    std::string orderID;
    std::getline(ss, orderID);

    return order_book.del(orderID) ? "OK" : "ERROR";
}
std::string OrderBookParser::modify(std::string& parameters)
{
    // MODIFY OrderId Price Quantity
    //  E.g.: MODIFY 1 2 2

    std::stringstream ss{parameters};
    std::string orderID, price_s, quantity_s;
    std::getline(ss, orderID, ' ');
    std::getline(ss, price_s, ' ');
    std::getline(ss, quantity_s);

    return order_book.modify(orderID, stoul(price_s), stoul(quantity_s)) ?
      "OK" : "ERROR";
}
std::string OrderBookParser::get(std::string& parameters)
{
    // GET OrderId
    //  E.g.: GET 1

    std::stringstream ss{parameters};
    std::string orderID;
    std::getline(ss, orderID);

    try
    {
        auto order = order_book.get(orderID);
        return "OK: " + order.to_string();
    }
    catch(...)
    {

    }

    return "ERROR";
}
std::string OrderBookParser::aggregated_best(std::string& parameters)
{
    // AGGREGATED_BEST ProductID
    //  E.g.: AGGREGATED_BEST 1

    std::stringstream ss{parameters};
    std::string productID;
    std::getline(ss, productID);

    std::string to_return;
    uint32_t bid_quantity, bid_price, ask_quantity, ask_price;
    if (order_book.aggregated_best(productID, bid_quantity, bid_price, 
      ask_quantity, ask_price))
    {   // OK0@1|0@0 => ERRORE, ma corretto dopo la CREATE.
        to_return = "OK: "+std::to_string(bid_quantity)+"@"
          +std::to_string(bid_price)+"|"+std::to_string(ask_quantity)+"@"
          +std::to_string(ask_price);  
    }
    else
    {
        to_return = "ERROR";
    }

    return to_return;
}


int main()
{
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