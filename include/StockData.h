#pragma once

#include <string>
#include <vector>

enum class TrendType {
    Bullish,
    Neutral,
    Bearish
};

enum class LimitStatus {
    None,
    LimitUp,
    LimitDown
};

struct StockData {
    std::string name = "Aurora Tech";
    std::string ticker = "AUR";
    std::string industry = "Technology";

    double price = 120.0;
    double previousPrice = 120.0;
    double openPrice = 120.0;
    double highPrice = 120.0;
    double lowPrice = 120.0;

    double changePercent = 0.0;
    double volatility = 0.0012;
    double momentum = 0.0;

    int volume = 806884;

    TrendType trend = TrendType::Neutral;

    std::vector<double> history;

    LimitStatus limitStatus = LimitStatus::None;

    double noiseVolatility = 0.00012;
};
