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

// TODOs:
// - input validation (e.g. min/max price/quantity/productID_length/ecc.);

#pragma once

#include <string>
#include <unordered_map>
#include <map>
#include <stdexcept>
#include <limits> // For checking overflow.
#include <sstream> // To build string efficiently, instead of concatenation.
#include <shared_mutex> // For shared_mutex.


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
    //  The key value is const by default in map.
    std::unordered_map<std::string, std::map<uint32_t, uint32_t>> bids;
    std::unordered_map<std::string, std::map<uint32_t, uint32_t>> asks;
    // Mutex made mutable, so it can be used in read-only methods.
    mutable std::shared_mutex m_shared_mutex; 

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
    // - limits the number of orders.

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



/*
** LOGGING.
*/
#include <stdexcept>
#include <iostream> // or proper logging framework

class OrderBookException : public std::runtime_error {
public:
    OrderBookException(const std::string& msg) : std::runtime_error(msg) {}
};

class OrderBook {
private:
    void log_operation(const std::string& operation, const std::string& orderID, bool success) {
        // In production: use spdlog, glog, or similar
        std::cout << "[" << get_timestamp() << "] " << operation 
                  << " orderID=" << orderID << " success=" << success << std::endl;
    }

    std::string get_timestamp() const {
        // Return current timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        return std::ctime(&time_t);
    }
};

/*
** STATISTICS & METRICS
*/
struct OrderBookStats {
    size_t total_orders;
    size_t buy_orders;
    size_t sell_orders;
    uint32_t highest_bid;
    uint32_t lowest_ask;
    uint32_t spread;
    uint64_t total_bid_volume;
    uint64_t total_ask_volume;
};

class OrderBook {
public:
    OrderBookStats get_statistics(const std::string& productID) const;
    std::vector<std::pair<uint32_t, uint32_t>> get_market_depth(
        const std::string& productID, size_t levels = 10) const;
};

#include <chrono>

class OrderBook {
private:
    struct Metrics {
        std::atomic<uint64_t> total_orders{0};
        std::atomic<uint64_t> create_operations{0};
        std::atomic<uint64_t> delete_operations{0};
        std::atomic<uint64_t> modify_operations{0};
        std::atomic<uint64_t> failed_operations{0};
    } metrics_;

public:
    const Metrics& get_metrics() const { return metrics_; }

    bool create(const std::string& orderID, const std::string& productID, 
                Order::Verb verb, uint32_t price, uint32_t quantity) {
        auto start = std::chrono::high_resolution_clock::now();

        bool result = create_impl(orderID, productID, verb, price, quantity);

        metrics_.create_operations++;
        if (!result) metrics_.failed_operations++;

        auto duration = std::chrono::high_resolution_clock::now() - start;
        log_performance("create", duration);

        return result;
    }
};

/*
** Market Data Interface.
*/
class MarketDataListener {
public:
    virtual ~MarketDataListener() = default;
    virtual void on_order_added(const Order& order) = 0;
    virtual void on_order_removed(const std::string& orderID) = 0;
    virtual void on_order_modified(const Order& old_order, const Order& new_order) = 0;
    virtual void on_best_bid_ask_changed(const std::string& productID, 
                                       uint32_t bid_price, uint32_t ask_price) = 0;
};

class OrderBook {
private:
    std::vector<std::shared_ptr<MarketDataListener>> listeners_;

public:
    void add_listener(std::shared_ptr<MarketDataListener> listener) {
        listeners_.push_back(listener);
    }

private:
    void notify_order_added(const Order& order) {
        for (auto& listener : listeners_) {
            listener->on_order_added(order);
        }
    }
};