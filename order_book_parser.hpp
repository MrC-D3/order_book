#pragma once

#include <string>
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
    {
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