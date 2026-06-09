#pragma once

#include "StockData.h"

#include <string>

enum class NewsSentiment {
    Positive,
    Negative,
    Neutral
};

struct MarketNews {
    std::string headline;
    std::string industry;
    NewsSentiment sentiment = NewsSentiment::Neutral;
    TrendType trendBias = TrendType::Neutral;
    double volatilityMultiplier = 1.0;
    int daysRemaining = 0;
};
