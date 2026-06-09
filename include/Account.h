#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include <unordered_map>
#include <vector>

struct TradeRecord {
    bool isBuy;
    std::string ticker;
    double price;
    double quantity;
    std::string time;
};

struct Position {
    double shares = 0.0;
    double averageCost = 0.0;
};

struct Account {
    double initialCash = 20000.0;
    double cash = 20000.0;
    std::unordered_map<std::string, Position> positions;
    std::vector<TradeRecord> trades;
};

#endif