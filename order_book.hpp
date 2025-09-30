// Notes:
// _ Use const in read-only parameters for better readability, compiler 
//  optimizations and potentially wrong usage.

#pragma once

#include <string>
#include <unordered_map>
#include <map>
#include <stdexcept>
#include <limits> // For checking overflow.
#include <sstream> // To build string efficiently, instead of concatenation.

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
    uint32_t price; // Oil futures had a negative price in 2020 for one day.
    uint32_t quantity;

    std::string to_string() const // Even better if overwrite operator<<().
    {
        // Using operator+ creates lots of temporary strings: expensive!
        std::ostringstream oss;
        oss << orderID << " " << productID << " " << 
          (verb == Verb::BUY ? "BUY" : "SELL")
          << " " << price << " " << quantity;

        return oss.str();
    }
};


class OrderBook
{
  public:
    // CRUD operations.
    bool create(const std::string& orderID, const std::string& productID, 
      const Order::Verb verb, const uint32_t price, const uint32_t quantity);
    bool del(const std::string& orderID);
    bool modify(const std::string& orderID, const uint32_t price, 
      const uint32_t quantity);
    const Order& get(const std::string& orderID); // Make it return bool.
    bool aggregated_best(const std::string& productID, uint32_t& bid_quantity, 
      uint32_t& bid_price, uint32_t& ask_quantity, uint32_t& ask_price);

  private:
    std::unordered_map<std::string, Order> orders;
    // Maps productID => {price, tot_quantity}.
    // The key value is const by default in map.
    std::unordered_map<std::string, std::map<uint32_t, uint32_t>> bids;
    std::unordered_map<std::string, std::map<uint32_t, uint32_t>> asks;

    void increase_quantity(const Order& order, 
      std::unordered_map<std::string, std::map<uint32_t, uint32_t>>& to_update);
    void decrease_quantity(const Order& order, 
      std::unordered_map<std::string, std::map<uint32_t, uint32_t>>& to_update);
};

void OrderBook::increase_quantity(const Order& order, 
      std::unordered_map<std::string, std::map<uint32_t, uint32_t>>& to_update)
{
    auto& prices = to_update[order.productID];

    if (std::numeric_limits<uint32_t>::max() - prices[order.price] < 
      order.quantity)
    {
        throw std::out_of_range{"Quantity overflow."};
    }
    prices[order.price] += order.quantity;
}
void OrderBook::decrease_quantity(const Order& order, 
      std::unordered_map<std::string, std::map<uint32_t, uint32_t>>& to_update)
{
    auto it_product = to_update.find(order.productID);
    if (it_product == to_update.end())
    {
        throw std::out_of_range{"ProductID doesn't exist."};
    }

    auto& prices = it_product->second;
    auto it_price = prices.find(order.price);
    if (it_price == prices.end())
    {
        throw std::out_of_range{"Price doesn't exist."};
    }

    auto& quantity = it_price->second;
    quantity -= order.quantity;
    if (quantity == 0) // It won't never be <0.
    {
        prices.erase(it_price);
    }
}

bool OrderBook::create(const std::string& orderID, const std::string& productID, 
  const Order::Verb verb, const uint32_t price, const uint32_t quantity)
{
    // TODO:
    // _ limits the number of orders.

    if (orders.find(orderID) != orders.end())
    {
        return false;
    }
    if (orderID == "" || productID == "")
    {
        return false;
    }

    Order new_order;
    new_order.orderID = orderID;
    new_order.productID = productID;
    new_order.verb = verb;
    new_order.price = price;
    new_order.quantity = quantity;

    orders[orderID] = new_order;

    // Increase bids OR asks.
    if (new_order.verb == Order::Verb::BUY)
    {
        increase_quantity(new_order, bids);
    }
    else
    {
        increase_quantity(new_order, asks);
    }
    
    return true;
}
bool OrderBook::del(const std::string& orderID)
{
    auto it = orders.find(orderID);
    if (it == orders.end())
    {
        return false;
    }

    // Decrease bids OR asks.
    if (it->second.verb == Order::Verb::BUY)
    {
        decrease_quantity(it->second, bids);
    }
    else
    {
        decrease_quantity(it->second, asks);
    }

    orders.erase(it);

    return true;
}
bool OrderBook::modify(const std::string& orderID, const uint32_t price, 
  const uint32_t quantity)
{
    if (orders.find(orderID) == orders.end())
    {
        return false;
    }

    auto& order = orders[orderID];

    if (price == order.price && quantity == order.quantity)
    {
        return true;
    }

    // Decrease bids OR asks.
    if (order.verb == Order::Verb::BUY)
    {
        decrease_quantity(order, bids);
    }
    else
    {
        decrease_quantity(order, asks);
    }

    // Finally update order.
    order.price = price;
    order.quantity = quantity;

    // Increase bids OR asks.
    if (order.verb == Order::Verb::BUY)
    {
        increase_quantity(order, bids);
    }
    else
    {
        increase_quantity(order, asks);
    }

    return true;
}
const Order& OrderBook::get(const std::string& orderID)
{
    auto it = orders.find(orderID);
    if (it == orders.end())
    {
        throw std::out_of_range{"orderID doesn't exist!"};
    }

    return it->second;
}
bool OrderBook::aggregated_best(const std::string& productID, uint32_t& bid_quantity, 
  uint32_t& bid_price, uint32_t& ask_quantity, uint32_t& ask_price)
{
    auto it_bid = bids.find(productID);
    auto it_ask = asks.find(productID);
    if (it_bid == bids.end() && it_ask == asks.end())
    {
        return false;
    }

    // To buy.
    if (it_bid == bids.end())
    {
        bid_quantity = 0;
        bid_price = 0;
    }
    else
    {
        // Increasing order, so the best bid is the last one.
        auto it_price = it_bid->second.rbegin();
        bid_quantity = it_price->second;
        bid_price = it_price->first;
    }

    // To sell.
    if (it_ask == asks.end())
    {
        ask_quantity = 0;
        ask_price = 0;
    }
    else
    {
        // Increasing order, so the best ask is the first one.
        auto it_price = it_ask->second.begin();
        ask_quantity = it_price->second;
        ask_price = it_price->first;
    }

    return true;
}
