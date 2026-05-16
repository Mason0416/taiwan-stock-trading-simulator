#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include <vector>

struct TradeRecord {

    bool isBuy;

    double price;

    double quantity;

    std::string time;
};

struct Account {

    double initialCash = 20000.0;

    double cash = 20000.0;

    double shares = 0.0;

    std::vector<TradeRecord> trades;
};

#endif