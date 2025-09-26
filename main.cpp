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

#include <iostream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <list>
#include <queue>
#include <map>


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
    std::string aggregated_best(std::string& parameters);

  private:
    std::unordered_map<std::string, Order> orders;
    std::unordered_map<std::string, std::map<uint32_t, uint32_t>> to_buy;
    std::unordered_map<std::string, std::map<uint32_t, uint32_t>> to_sell;

    void increase_quantity(Order& order, 
      std::unordered_map<std::string, std::map<uint32_t, uint32_t>>& to_update);
    void decrease_quantity(Order& order, 
      std::unordered_map<std::string, std::map<uint32_t, uint32_t>>& to_update);
};

void OrderBook::increase_quantity(Order& order, 
      std::unordered_map<std::string, std::map<uint32_t, uint32_t>>& to_update)
{
    // What if keys don't exist?
    auto& prices = to_update[order.productID];
    prices[order.price] += order.quantity;
}
void OrderBook::decrease_quantity(Order& order, 
      std::unordered_map<std::string, std::map<uint32_t, uint32_t>>& to_update)
{
    // What if keys don't exist?
    auto& prices = to_update[order.productID];
    prices[order.price] -= order.quantity;

    // Gestione delle quantità zero: Le funzioni decrease_quantity possono 
    //  portare a quantità negative o zero, ma il codice non rimuove le entry 
    //  con quantità zero dalle mappe, causando accumulo di dati non validi.

}

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

    // Increase to_buy OR to_sell.
    if (new_order.verb == Order::Verb::BUY)
    {
        increase_quantity(new_order, to_buy);
    }
    else
    {
        increase_quantity(new_order, to_sell);
    }
    
    return "OK";
}

std::string OrderBook::del(std::string& parameters)
{
    std::stringstream ss{parameters};
    // No need to call clear() since it's a new stream.
    std::string orderID;
    std::getline(ss, orderID);
    
    auto it = orders.find(orderID);
    if (it == orders.end())
    {
        return "ERROR";
    }

    // Decrease to_buy OR to_sell.
    if (it->second.verb == Order::Verb::BUY)
    {
        decrease_quantity(it->second, to_buy);
    }
    else
    {
        decrease_quantity(it->second, to_sell);
    }

    orders.erase(it);

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

    // Decrease to_buy OR to_sell.
    if (order.verb == Order::Verb::BUY)
    {
        decrease_quantity(order, to_buy);
    }
    else
    {
        decrease_quantity(order, to_sell);
    }

    // Finally update order.
    order.price = price;
    order.quantity = quantity;

    // Increase to_buy OR to_sell.
    if (order.verb == Order::Verb::BUY)
    {
        increase_quantity(order, to_buy);
    }
    else
    {
        increase_quantity(order, to_sell);
    }

    std::cout << "  Modified: " << order.to_string() << "\n";
    return "OK";
}

std::string OrderBook::get(std::string& parameters)
{
    std::stringstream ss{parameters};
    std::string orderID;
    std::getline(ss, orderID);
    if (orders.find(orderID) == orders.end())
    {
        return "ERROR";
    }

    auto& order = orders[orderID];
    return "OK " + order.to_string();
}

std::string OrderBook::aggregated_best(std::string& parameters)
{
    std::stringstream ss{parameters};
    std::string productID;
    std::getline(ss, productID);

    auto it_buy = to_buy.find(productID);
    auto it_sell = to_sell.find(productID);
    if (it_buy == to_buy.end() && it_sell == to_sell.end())
    {
        return "ERROR";
    }
    std::string to_return{"OK:"};

    // To buy.
    if (it_buy == to_buy.end())
    {
        to_return += "0@0|";
    }
    else
    {
        auto it_price = it_buy->second.begin();
        to_return += std::to_string(it_price->second) + "@" + 
          std::to_string(it_price->first) + "|";
    }

    // To sell.
    if (it_sell == to_sell.end())
    {
        to_return += "0@0";
    }
    else
    {
        auto it_price = it_sell->second.rbegin();
        to_return += std::to_string(it_price->second) + "@" + 
          std::to_string(it_price->first);
    }

    return to_return;
}


int main()
{
    OrderBook order_book;

    // Mancanza di validazione input.

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