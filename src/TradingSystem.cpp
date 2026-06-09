#include "TradingSystem.h"
#include "GUI.h"
#include "StockData.h"
#include "Account.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {
std::string SentimentName(NewsSentiment sentiment) {
    switch (sentiment) {
        case NewsSentiment::Positive: return "Positive";
        case NewsSentiment::Negative: return "Negative";
        default: return "Neutral";
    }
}

Color SentimentColor(NewsSentiment sentiment) {
    switch (sentiment) {
        case NewsSentiment::Positive: return Theme::UP_RED;
        case NewsSentiment::Negative: return Theme::DOWN_GREEN;
        default: return Theme::TEXT;
    }
}

std::string TrendBiasLabel(TrendType trendBias) {
    switch (trendBias) {
        case TrendType::Bullish: return "Bullish Pressure";
        case TrendType::Bearish: return "Bearish Pressure";
        default: return "Neutral Outlook";
    }
}

Color TrendBiasColor(TrendType trendBias) {
    switch (trendBias) {
        case TrendType::Bullish: return Theme::UP_RED;
        case TrendType::Bearish: return Theme::DOWN_GREEN;
        default: return Theme::TEXT;
    }
}

std::string NewsVolatilityLabel(double multiplier) {
    if (multiplier >= 1.55) {
        return "High";
    }
    if (multiplier >= 1.30) {
        return "Medium";
    }
    return "Low";
}

Color NewsVolatilityColor(double multiplier) {
    if (multiplier >= 1.55) {
        return Theme::UP_RED;
    }
    if (multiplier >= 1.30) {
        return Color{224, 154, 55, 255};
    }
    return Theme::MUTED;
}
}

TradingSystem::TradingSystem()
    : rng(std::random_device{}()) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Raylib Stock Trading");
    SetWindowMinSize(minScreenWidth, minScreenHeight);
    SetTargetFPS(60);

    SetupFont();
    InitStocks();
    GenerateNews();
    PickTrendForNewDay();
    for (auto& stock : stocks) {
        SeedInitialHistory(stock);
    }
    SetupUI();
}

TradingSystem::~TradingSystem() {
    UnloadFontSafe();
    CloseWindow();
}

void TradingSystem::SeedInitialHistory(StockData& stock) {
    stock.history.clear();

    double price = stock.openPrice;
    std::normal_distribution<double> seedNoise(0.0, stock.openPrice * 0.0012);

    for (int i = 0; i < 42; ++i) {
        double drift = 0.0;

        if (stock.trend == TrendType::Bullish) {
            drift = stock.openPrice * 0.0008;
        } else if (stock.trend == TrendType::Bearish) {
            drift = -stock.openPrice * 0.0008;
        }

        price = std::clamp(
            price + drift + seedNoise(rng),
            GetLimitDownPrice(stock),
            GetLimitUpPrice(stock)
        );
        stock.history.push_back(price);
    }

    stock.price = stock.history.back();
    stock.previousPrice = stock.history.size() >= 2
        ? stock.history[stock.history.size() - 2]
        : stock.price;
    stock.highPrice = *std::max_element(stock.history.begin(), stock.history.end());
    stock.lowPrice = *std::min_element(stock.history.begin(), stock.history.end());
    stock.changePercent = ((stock.price - stock.openPrice) / stock.openPrice) * 100.0;
    stock.volatility = stock.openPrice > 0.0
        ? ((stock.highPrice - stock.lowPrice) / stock.openPrice) * 100.0
        : 0.0;
    stock.momentum = stock.changePercent;
}

void TradingSystem::Run() {
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        Update(dt);

        BeginDrawing();
        ClearBackground(Theme::BG);
        Draw();
        EndDrawing();
    }
}

void TradingSystem::InitStocks() {
    stocks = {
        {"Aurora Tech", "AUR", "Technology", 120.0, 120.0, 120.0, 120.0, 120.0, 0.0, 0.0012, 0.0, 806884, TrendType::Neutral, {120.0}},
        {"Nova Bank", "NVB", "Finance", 65.5, 65.5, 65.5, 65.5, 65.5, 0.0, 0.0015, 0.0, 540712, TrendType::Neutral, {65.5}},
        {"MedCore Health", "MCH", "Healthcare", 88.3, 88.3, 88.3, 88.3, 88.3, 0.0, 0.0011, 0.0, 427901, TrendType::Neutral, {88.3}},
        {"Green Energy", "GEE", "Energy", 52.1, 52.1, 52.1, 52.1, 52.1, 0.0, 0.0016, 0.0, 639284, TrendType::Neutral, {52.1}},
        {"Ocean Shipping", "OSC", "Transportation", 36.7, 36.7, 36.7, 36.7, 36.7, 0.0, 0.0014, 0.0, 312604, TrendType::Neutral, {36.7}}
    };

    selectedStock = 0;
}

void TradingSystem::SetupUI() {
    UpdateLayout();
}

void TradingSystem::UpdateLayout() {
    const float ui = GetUIScale();
    const float sw = static_cast<float>(std::max(GetScreenWidth(), minScreenWidth));
    const float sh = static_cast<float>(std::max(GetScreenHeight(), minScreenHeight));
    const float margin = 40.0f;
    const float gap = 20.0f;

    header = {0.0f, 0.0f, sw, 92.0f};
    marketRow = {0.0f, header.height, sw, 82.0f};

    const float bottomHeight = 70.0f;
    const float contentTop = marketRow.y + marketRow.height + 22.0f;
    const float contentBottom = sh - bottomHeight - 30.0f;
    const float contentHeight = std::max(460.0f, contentBottom - contentTop);
    const float availableWidth = sw - margin * 2.0f - gap * 2.0f;

    const float leftWidth = availableWidth * 0.20f;
    const float chartWidth = availableWidth * 0.55f;
    const float rightWidth = availableWidth - leftWidth - chartWidth;

    leftPanel = {
        margin,
        contentTop,
        leftWidth,
        contentHeight
    };

    chartPanel = {
        leftPanel.x + leftPanel.width + gap,
        contentTop,
        chartWidth,
        contentHeight
    };

    rightPanel = {
        chartPanel.x + chartPanel.width + gap,
        contentTop,
        rightWidth,
        contentHeight
    };

    bottomPanel = {
        margin,
        sh - bottomHeight - 18.0f,
        sw - margin * 2.0f,
        bottomHeight
    };

    const int dropdownIndex = stockDropdown.GetSelectedIndex();
    stockDropdown = StockDropdown({
        leftPanel.x + 20.0f,
        leftPanel.y + 76.0f * ui,
        leftPanel.width - 40.0f,
        std::min(50.0f, 42.0f * ui)
    });

    std::vector<std::string> names;
    for (const auto& stock : stocks) {
        names.push_back(stock.name + " (" + stock.ticker + ")");
    }

    stockDropdown.SetOptions(names);
    stockDropdown.SetSelectedIndex(std::max(0, dropdownIndex));
    if (!stocks.empty()) {
        int statusIndex = std::clamp(selectedStock, 0, static_cast<int>(stocks.size()) - 1);
        stockDropdown.SetLimitStatus(GetLimitStatus(stocks[statusIndex]));
    }

    const float panelX = rightPanel.x + 25.0f;
    const float fullWidth = rightPanel.width - 50.0f;
    const float halfWidth = (fullWidth - 10.0f) * 0.5f;
    const float tradeTop = rightPanel.y + std::clamp(rightPanel.height * 0.39f, 210.0f, 292.0f);
    const float buttonHeight = 42.0f;
    const float quantityY = tradeTop + 84.0f;
    const float actionY = std::min(
        quantityY + 112.0f * ui,
        rightPanel.y + rightPanel.height - 122.0f * ui
    );

    wholeButton = Button(
        {panelX, tradeTop, halfWidth, buttonHeight},
        "Board Lot",
        Theme::GRAY_BUTTON,
        Theme::GRAY_HOVER,
        Theme::TEXT
    );

    fractionalButton = Button(
        {panelX + halfWidth + 10.0f, tradeTop, halfWidth, buttonHeight},
        "Odd Lot",
        Theme::GRAY_BUTTON,
        Theme::GRAY_HOVER,
        Theme::TEXT
    );

    quantityInputBox = {
        panelX + 55.0f,
        quantityY,
        std::max(120.0f, fullWidth - 110.0f),
        buttonHeight
    };

    minusButton = Button(
        {panelX, quantityY, 45.0f, buttonHeight},
        "-",
        Theme::GRAY_BUTTON,
        Theme::GRAY_HOVER,
        Theme::TEXT
    );

    plusButton = Button(
        {panelX + fullWidth - 45.0f, quantityY, 45.0f, buttonHeight},
        "+",
        Theme::GRAY_BUTTON,
        Theme::GRAY_HOVER,
        Theme::TEXT
    );

    buyButton = Button(
        {panelX, actionY, halfWidth, buttonHeight},
        "Buy",
        Theme::UP_RED,
        Color{230, 86, 95, 255},
        WHITE
    );

    sellButton = Button(
        {panelX + halfWidth + 10.0f, actionY, halfWidth, buttonHeight},
        "Sell",
        Theme::DOWN_GREEN,
        Color{26, 184, 120, 255},
        WHITE
    );

    nextDayButton = Button(
        {panelX, actionY + 52.0f, fullWidth, buttonHeight},
        "Next Day",
        Theme::GRAY_BUTTON,
        Theme::GRAY_HOVER,
        Theme::TEXT
    );

    newsButton = Button(
        {360.0f, 25.0f, 145.0f, 42.0f},
        "News History",
        Theme::GRAY_BUTTON,
        Theme::GRAY_HOVER,
        Theme::TEXT
    );

    avatarButton = Button(
        {sw - 76.0f, 18.0f, 56.0f, 56.0f},
        "A",
        Theme::ACCENT_BLUE,
        Theme::ACCENT_BLUE_HOVER,
        WHITE
    );

    backButton = Button(
        {40.0f, 25.0f, 110.0f, 42.0f},
        "Back",
        Theme::GRAY_BUTTON,
        Theme::GRAY_HOVER,
        Theme::TEXT
    );

    historyButton = Button(
        {190.0f, 25.0f, 145.0f, 42.0f},
        "History",
        Theme::GRAY_BUTTON,
        Theme::GRAY_HOVER,
        Theme::TEXT
    );

    UpdateTradeModeButtons();
}

void TradingSystem::ToggleFullscreenMode() {
    if (!IsWindowFullscreen()) {
        windowedWidth = GetScreenWidth();
        windowedHeight = GetScreenHeight();

        const int monitor = GetCurrentMonitor();
        SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        ToggleFullscreen();
    } else {
        ToggleFullscreen();
        SetWindowSize(
            std::max(windowedWidth, minScreenWidth),
            std::max(windowedHeight, minScreenHeight)
        );
    }

    UpdateLayout();
}

void TradingSystem::Update(float dt) {
    if (IsKeyPressed(KEY_F11)) {
        ToggleFullscreenMode();
    }

    if (IsWindowResized()) {
        UpdateLayout();
    }

    if (page == Page::Main) {
        UpdateMain(dt);
    } else if (page == Page::Account) {
        UpdateAccount(dt);
    } else if (page == Page::History) {
        UpdateHistory(dt);
    } else {
        UpdateNewsHistory(dt);
    }
}

void TradingSystem::UpdateMain(float dt) {
    stockDropdown.Update();
    selectedStock = stockDropdown.GetSelectedIndex();
    stockDropdown.SetLimitStatus(GetLimitStatus(CurrentStock()));

    wholeButton.Update(dt);
    fractionalButton.Update(dt);
    buyButton.Update(dt);
    sellButton.Update(dt);
    nextDayButton.Update(dt);
    minusButton.Update(dt);
    plusButton.Update(dt);
    avatarButton.Update(dt);

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointRec(GetMousePosition(), avatarButton.GetBounds())) {
        page = Page::Account;
        return;
    }

    if (wholeButton.IsClicked()) {
        tradeMode = TradeMode::WholeShare;
        orderQuantity = std::max(1.0, std::round(orderQuantity));
        quantityInput = std::to_string(static_cast<int>(orderQuantity));
        UpdateTradeModeButtons();
    }

    if (fractionalButton.IsClicked()) {
        tradeMode = TradeMode::OddLotShare;
        orderQuantity = std::max(1.0, std::round(orderQuantity));
        quantityInput = std::to_string(static_cast<int>(orderQuantity));
        UpdateTradeModeButtons();
    }

    if (plusButton.IsClicked()) {
        orderQuantity += 1.0;
        quantityInput = std::to_string(static_cast<int>(orderQuantity));
    }

    if (minusButton.IsClicked()) {
        orderQuantity = std::max(1.0, orderQuantity - 1.0);
        quantityInput = std::to_string(static_cast<int>(orderQuantity));
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        editingQuantity = CheckCollisionPointRec(GetMousePosition(), quantityInputBox);
    }

    if (editingQuantity) {
        int key = GetCharPressed();

        while (key > 0) {
            char c = static_cast<char>(key);

            if (c >= '0' && c <= '9') {
                quantityInput.push_back(c);
            }

            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && !quantityInput.empty()) {
            quantityInput.pop_back();
        }

        if (IsKeyPressed(KEY_ENTER)) {
            ApplyQuantityInput();
            editingQuantity = false;
        }
    }

    if (buyButton.IsClicked()) {
        Buy();
    }

    if (sellButton.IsClicked()) {
        Sell();
    }

    if (nextDayButton.IsClicked()) {
        NextDay();
    }

    AdvanceTime(dt);
    UpdatePrice(dt);
}

void TradingSystem::ApplyQuantityInput() {
    if (quantityInput.empty()) {
        return;
    }

    try {
        bool integerInput = std::all_of(quantityInput.begin(), quantityInput.end(), [](char c) {
            return c >= '0' && c <= '9';
        });

        if (!integerInput) {
            quantityInput = std::to_string(static_cast<int>(orderQuantity));
            return;
        }

        double value = std::stod(quantityInput);
        value = std::max(1.0, std::round(value));

        orderQuantity = value;
        quantityInput = std::to_string(static_cast<int>(orderQuantity));
    } catch (...) {
        quantityInput = std::to_string(static_cast<int>(orderQuantity));
    }
}

void TradingSystem::UpdateAccount(float dt) {
    backButton.Update(dt);
    historyButton.Update(dt);
    newsButton.Update(dt);
    avatarButton.Update(dt);

    if (backButton.IsClicked()) {
        page = Page::Main;
    }

    if (historyButton.IsClicked()) {
        page = Page::History;
    }

    if (newsButton.IsClicked()) {
        page = Page::NewsHistory;
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointRec(GetMousePosition(), avatarButton.GetBounds())) {
        page = Page::Main;
    }
}

void TradingSystem::UpdateHistory(float dt) {
    backButton.Update(dt);
    avatarButton.Update(dt);

    if (backButton.IsClicked()) {
        page = Page::Account;
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointRec(GetMousePosition(), avatarButton.GetBounds())) {
        page = Page::Main;
    }
}

void TradingSystem::UpdateTradeModeButtons() {
    wholeButton.SetActive(tradeMode == TradeMode::WholeShare);
    fractionalButton.SetActive(tradeMode == TradeMode::OddLotShare);
}

void TradingSystem::AdvanceTime(float dt) {
    timeAccumulator += dt;

    if (timeAccumulator < 1.0f) {
        return;
    }

    timeAccumulator -= 1.0f;

    second++;

    if (second >= 60) {
        second = 0;
        minute++;
    }

    if (minute >= 60) {
        minute = 0;
        hour++;
    }

    if (hour >= 17) {
        NextDay();
    }
}

void TradingSystem::UpdatePrice(float dt) {
    priceAccumulator += dt;

    if (priceAccumulator < 1.0f) {
        return;
    }

    priceAccumulator -= 1.0f;

    for (auto& stock : stocks) {
        stock.previousPrice = stock.price;
        const double limitUpPrice = GetLimitUpPrice(stock);
        const double limitDownPrice = GetLimitDownPrice(stock);

        auto recordTick = [&]() {
            stock.highPrice = std::max(stock.highPrice, stock.price);
            stock.lowPrice = std::min(stock.lowPrice, stock.price);
            stock.changePercent = ((stock.price - stock.openPrice) / stock.openPrice) * 100.0;
            stock.momentum = ((stock.price - stock.previousPrice) / stock.previousPrice) * 100.0;
            stock.volatility = stock.openPrice > 0.0
                ? ((stock.highPrice - stock.lowPrice) / stock.openPrice) * 100.0
                : 0.0;
            stock.volume += RandomInt(1800, 5200);

            stock.history.push_back(stock.price);
            if (stock.history.size() > 90) {
                stock.history.erase(stock.history.begin());
            }
        };

        if (stock.limitStatus == LimitStatus::LimitUp ||
            stock.limitStatus == LimitStatus::LimitDown) {
            const bool unlockForTrading = RandomDouble(0.0, 1.0) < 0.02;

            if (!unlockForTrading) {
                stock.price = stock.limitStatus == LimitStatus::LimitUp
                    ? limitUpPrice
                    : limitDownPrice;
                recordTick();
                continue;
            }

            stock.limitStatus = LimitStatus::None;
        }

        const double trendDrift = stock.price * GetTrendDriftPercent(stock.trend);
        const double randomNoise = stock.price * GetRandomNoisePercent(stock);
        const double newsAdjustment = stock.price * GetNewsAdjustmentPercent(stock);

        double nextPrice = stock.price + trendDrift + randomNoise + newsAdjustment;

        if (nextPrice >= limitUpPrice) {
            stock.price = limitUpPrice;
            stock.limitStatus = LimitStatus::LimitUp;
        } else if (nextPrice <= limitDownPrice) {
            stock.price = limitDownPrice;
            stock.limitStatus = LimitStatus::LimitDown;
        } else {
            stock.price = std::max(1.0, nextPrice);
            stock.limitStatus = LimitStatus::None;
        }

        recordTick();
    }

    UpdateMarketIndex();
}

void TradingSystem::PickTrendForNewDay() {
    for (auto& stock : stocks) {
        double bullishWeight = 30.0;
        double neutralWeight = 40.0;
        double bearishWeight = 30.0;

        const double newsInfluence = GetNewsInfluenceForStock(stock);
        const double trendBiasDirection = GetTrendBiasDirection(stock);

        if (trendBiasDirection > 0.0) {
            bullishWeight += 12.0 * newsInfluence;
            neutralWeight -= 3.0 * newsInfluence;
            bearishWeight -= 9.0 * newsInfluence;
        } else if (trendBiasDirection < 0.0) {
            bullishWeight -= 9.0 * newsInfluence;
            neutralWeight -= 3.0 * newsInfluence;
            bearishWeight += 12.0 * newsInfluence;
        }

        bullishWeight = std::max(1.0, bullishWeight);
        neutralWeight = std::max(1.0, neutralWeight);
        bearishWeight = std::max(1.0, bearishWeight);

        std::discrete_distribution<int> trendDist({
            bullishWeight,
            neutralWeight,
            bearishWeight
        });

        int choice = trendDist(rng);

        if (choice == 0) {
            stock.trend = TrendType::Bullish;
        } else if (choice == 1) {
            stock.trend = TrendType::Neutral;
        } else {
            stock.trend = TrendType::Bearish;
        }

        const double normalNoise = RandomDouble(0.00008, 0.00022);
        double targetNoise = normalNoise;

        if (!activeNews.empty() && activeNews.front().daysRemaining > 0) {
            const double eventMultiplier = activeNews.front().volatilityMultiplier;
            const double volatilityStrength = std::clamp((eventMultiplier - 1.0) / 0.6, 0.0, 1.0);
            const double highNewsNoise = RandomDouble(0.00022, 0.00045);
            targetNoise += (highNewsNoise - normalNoise) * newsInfluence * volatilityStrength;
        }

        stock.noiseVolatility = std::clamp(targetNoise, 0.00008, 0.00045);
    }
}

void TradingSystem::UpdateMarketIndex() {
    static const std::vector<std::string> industries = {
        "Technology",
        "Finance",
        "Healthcare",
        "Energy",
        "Transportation"
    };

    double industryReturnSum = 0.0;
    int industryCount = 0;

    for (const auto& industry : industries) {
        double stockReturnSum = 0.0;
        int stockCount = 0;

        for (const auto& stock : stocks) {
            if (stock.industry != industry || stock.previousPrice <= 0.0) {
                continue;
            }

            stockReturnSum += (stock.price - stock.previousPrice) / stock.previousPrice;
            stockCount++;
        }

        if (stockCount > 0) {
            industryReturnSum += stockReturnSum / static_cast<double>(stockCount);
            industryCount++;
        }
    }

    if (industryCount == 0) {
        return;
    }

    const double averageReturn = industryReturnSum / static_cast<double>(industryCount);
    const double indexDamping = 0.22;
    const double indexReturn = averageReturn * indexDamping + GetMarketNewsIndexDrift();

    marketIndex = std::max(1.0, marketIndex * (1.0 + indexReturn));
}

double TradingSystem::GetMarketNewsIndexDrift() const {
    if (activeNews.empty() || activeNews.front().daysRemaining <= 0) {
        return 0.0;
    }

    const MarketNews& news = activeNews.front();
    const double newsStrength = std::clamp((news.volatilityMultiplier - 1.0) / 0.6, 0.0, 1.0);

    if (news.trendBias == TrendType::Bullish) {
        return 0.000003 * newsStrength;
    }

    if (news.trendBias == TrendType::Bearish) {
        return -0.000003 * newsStrength;
    }

    return 0.0;
}

double TradingSystem::GetTrendDriftPercent(TrendType trend) {
    switch (trend) {
        case TrendType::Bullish:
            return RandomDouble(0.000012, 0.000035);
        case TrendType::Bearish:
            return RandomDouble(-0.000035, -0.000012);
        default:
            return RandomDouble(-0.00001, 0.00001);
    }
}

double TradingSystem::GetRandomNoisePercent(const StockData& stock) {
    return RandomDouble(-stock.noiseVolatility, stock.noiseVolatility);
}

double TradingSystem::GetNewsAdjustmentPercent(const StockData& stock) {
    const double biasDirection = GetTrendBiasDirection(stock);
    if (std::abs(biasDirection) < 1e-6) {
        return 0.0;
    }

    const double influence = GetNewsInfluenceForStock(stock);
    return biasDirection * influence * RandomDouble(0.000008, 0.00002);
}

void TradingSystem::NextDay() {
    day++;
    hour = 9;
    minute = 0;
    second = 0;
    timeAccumulator = 0.0f;
    priceAccumulator = 0.0f;
    marketOpenIndex = marketIndex;

    if (!activeNews.empty()) {
        activeNews.front().daysRemaining--;
    }

    activeNews.erase(
        std::remove_if(activeNews.begin(), activeNews.end(), [](const MarketNews& news) {
            return news.daysRemaining <= 0;
        }),
        activeNews.end()
    );

    if (activeNews.empty()) {
        GenerateNews();
    }
    PickTrendForNewDay();

    for (auto& stock : stocks) {
        stock.openPrice = stock.price;
        stock.previousPrice = stock.price;
        stock.highPrice = stock.price;
        stock.lowPrice = stock.price;
        stock.changePercent = 0.0;
        stock.volatility = 0.0;
        stock.momentum = 0.0;
        stock.limitStatus = LimitStatus::None;

        stock.history.clear();
        stock.history.reserve(90);
        for (int i = 0; i < 40; ++i) {
            stock.history.push_back(stock.price);
        }
    }
}

void TradingSystem::Buy() {
    StockData& stock = CurrentStock();
    double cost = GetEstimatedCost();
    double qty = GetOrderShares();

    if (qty <= 0 || account.cash < cost) {
        return;
    }

    account.cash -= cost;
    Position& position = account.positions[stock.ticker];
    double totalCost = position.averageCost * position.shares + stock.price * qty;
    position.shares += qty;
    position.averageCost = (position.shares > 0.0) ? (totalCost / position.shares) : 0.0;

    account.trades.push_back({
        true,
        stock.ticker,
        stock.price,
        qty,
        TimeString()
    });
}

void TradingSystem::Sell() {
    StockData& stock = CurrentStock();
    double qty = GetOrderShares();
    auto it = account.positions.find(stock.ticker);
    if (qty <= 0 || it == account.positions.end() || it->second.shares < qty) {
        return;
    }

    account.cash += qty * stock.price;
    it->second.shares -= qty;
    if (it->second.shares <= 1e-6) {
        account.positions.erase(it);
    }

    account.trades.push_back({
        false,
        stock.ticker,
        stock.price,
        qty,
        TimeString()
    });
}

void TradingSystem::Draw() {
    if (page == Page::Main) {
        DrawMain();
    } else if (page == Page::Account) {
        DrawAccount();
    } else if (page == Page::History) {
        DrawHistory();
    } else {
        DrawNewsHistory();
    }
}

void TradingSystem::DrawMain() {
    DrawHeader("Stock Trading System", true);
    DrawMarketRow();
    DrawStockInfoPanel();
    DrawChartPanel();
    DrawTradingPanel();
    DrawBottomSummary();
}

void TradingSystem::DrawAccount() {
    DrawHeader("", false);

    const float sw = static_cast<float>(GetScreenWidth());
    const float sh = static_cast<float>(GetScreenHeight());
    const float margin = 40.0f;
    Rectangle panel = {
        margin,
        header.height + 34.0f,
        sw - margin * 2.0f,
        sh - header.height - 74.0f
    };
    DrawPanel(panel);

    const float pad = 30.0f;
    const float infoX = panel.x + pad;
    const float holdingsX = panel.x + panel.width * 0.38f;
    const float sharesX = holdingsX + panel.width * 0.17f;
    const float valueX = holdingsX + panel.width * 0.34f;
    const float topY = panel.y + pad;

    DrawUI("Account Info", {infoX, topY}, 24.0f, Theme::TEXT);

    double total = TotalAsset();
    double portfolio = GetPortfolioValue();
    double pnl = total - account.initialCash;
    double pnlPercent = pnl / account.initialCash * 100.0;

    float infoY = topY + 55.0f;
    DrawLabelValue("Player", "Trader", {infoX, infoY});
    infoY += 58.0f;
    DrawLabelValue("Initial Cash", FormatMoney(account.initialCash), {infoX, infoY});
    infoY += 58.0f;
    DrawLabelValue("Cash", FormatMoney(account.cash), {infoX, infoY});
    infoY += 58.0f;
    DrawLabelValue("Portfolio Value", FormatMoney(portfolio), {infoX, infoY});
    infoY += 58.0f;

    DrawLabelValue(
        "Total Asset",
        FormatMoney(total),
        {infoX, infoY},
        true,
        pnl >= 0.0 ? Theme::UP_RED : Theme::DOWN_GREEN
    );
    infoY += 58.0f;

    DrawLabelValue(
        "Total Return",
        FormatMoney(pnl) + " (" + FormatChangePercent(pnlPercent) + ")",
        {infoX, infoY},
        true,
        pnl >= 0.0 ? Theme::UP_RED : Theme::DOWN_GREEN
    );

    DrawUI("Holdings", {holdingsX, topY}, 24.0f, Theme::TEXT);
    DrawUI("Ticker", {holdingsX, topY + 45.0f}, 18.0f, Theme::MUTED);
    DrawUI("Shares", {sharesX, topY + 45.0f}, 18.0f, Theme::MUTED);
    DrawUI("Market Value", {valueX, topY + 45.0f}, 18.0f, Theme::MUTED);

    float y = topY + 80.0f;
    const float maxY = panel.y + panel.height - 38.0f;
    if (account.positions.empty()) {
        DrawUI("No active positions yet.", {holdingsX, y}, 18.0f, Theme::MUTED);
        y += 30.0f;
    } else {
        for (const auto& entry : account.positions) {
            if (y > maxY) break;

            const std::string& ticker = entry.first;
            const Position& position = entry.second;
            double price = 0.0;
            for (const auto& stock : stocks) {
                if (stock.ticker == ticker) {
                    price = stock.price;
                    break;
                }
            }
            DrawUI(ticker, {holdingsX, y}, 20.0f, Theme::TEXT);
            DrawNumber(FormatNumber(position.shares), {sharesX, y + 2.0f}, 20.0f, Theme::TEXT);
            DrawNumber(FormatMoney(position.shares * price), {valueX, y + 2.0f}, 20.0f, Theme::TEXT);
            y += 36.0f;
        }
    }

    backButton.Draw();
    historyButton.Draw();
    newsButton.Draw();
    avatarButton.Draw();
}

void TradingSystem::DrawHistory() {
    DrawHeader("", false);

    const float sw = static_cast<float>(GetScreenWidth());
    const float sh = static_cast<float>(GetScreenHeight());
    const float margin = 40.0f;
    Rectangle panel = {
        margin,
        header.height + 34.0f,
        sw - margin * 2.0f,
        sh - header.height - 74.0f
    };
    DrawPanel(panel);

    const float x = panel.x + 30.0f;
    const float topY = panel.y + 30.0f;

    DrawUI("Trade Records", {x, topY}, 24.0f, Theme::TEXT);

    if (account.trades.empty()) {
        DrawUI("No trade history yet.", {x, topY + 55.0f}, 18.0f, Theme::MUTED);
    } else {
        float y = topY + 55.0f;
        const float maxY = panel.y + panel.height - 38.0f;

        for (int i = static_cast<int>(account.trades.size()) - 1; i >= 0 && y < maxY; --i) {
            const TradeRecord& t = account.trades[i];

            std::ostringstream ss;
            ss << (t.isBuy ? "BUY  " : "SELL ")
               << t.ticker << " "
               << std::fixed << std::setprecision(2)
               << t.quantity << " @ "
               << FormatMoney(t.price)
               << "    " << t.time;

            DrawUI(
                ss.str(),
                {x, y},
                20.0f,
                t.isBuy ? Theme::UP_RED : Theme::DOWN_GREEN
            );

            y += 34.0f;
        }
    }

    backButton.Draw();
    avatarButton.Draw();
}

void TradingSystem::UpdateNewsHistory(float dt) {
    backButton.Update(dt);
    if (backButton.IsClicked()) {
        page = Page::Account;
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) &&
        CheckCollisionPointRec(GetMousePosition(), avatarButton.GetBounds())) {
        page = Page::Account;
        return;
    }
    avatarButton.Update(dt);
}

void TradingSystem::DrawNewsHistory() {
    DrawHeader("", false);

    const float sw = static_cast<float>(GetScreenWidth());
    const float sh = static_cast<float>(GetScreenHeight());
    const float margin = 40.0f;
    Rectangle panel = {
        margin,
        header.height + 34.0f,
        sw - margin * 2.0f,
        sh - header.height - 74.0f
    };
    DrawPanel(panel);

    const float x = panel.x + 30.0f;
    const float topY = panel.y + 30.0f;

    DrawUI("News History", {x, topY}, 24.0f, Theme::TEXT);
    DrawUI("See the latest market-moving headlines and industry impacts.", {x, topY + 40.0f}, 18.0f, Theme::MUTED);

    float y = topY + 75.0f;
    const float maxY = panel.y + panel.height - 38.0f;
    if (newsHistory.empty()) {
        DrawUI("No news events recorded yet.", {x, y}, 18.0f, Theme::MUTED);
    } else {
        for (const auto& entry : newsHistory) {
            if (y > maxY) break;

            std::ostringstream ss;
            ss << "Day " << entry.first << " | " << entry.second.industry
               << " | " << entry.second.headline;

            DrawUI(ss.str(), {x, y}, 20.0f, Theme::TEXT);
            DrawUI(
                "Sentiment: " + SentimentName(entry.second.sentiment) +
                " | Trend Bias: " + TrendBiasLabel(entry.second.trendBias) +
                " | Volatility: " + NewsVolatilityLabel(entry.second.volatilityMultiplier),
                {x, y + 24.0f},
                18.0f,
                SentimentColor(entry.second.sentiment)
            );
            y += 48.0f;
        }
    }

    backButton.Draw();
    avatarButton.Draw();
}

void TradingSystem::DrawHeader(const std::string& title, bool drawInfo) {
    const float ui = GetUIScale();
    DrawRectangleRec(header, Theme::TOP);

    DrawLineEx(
        {0.0f, header.height - 1.0f},
        {header.width, header.height - 1.0f},
        1.5f,
        Theme::BORDER_SOFT
    );

    DrawUI(title, {40.0f, 28.0f}, 36.0f * ui, Theme::TEXT);

    if (drawInfo) {
        const float infoX = std::max(420.0f, header.width * 0.42f);
        DrawUI("Day " + std::to_string(day), {infoX, 31.0f}, 21.0f * ui, Theme::MUTED);
        DrawUI(TimeString(), {infoX + 112.0f * ui, 31.0f}, 21.0f * ui, Theme::TEXT);
        DrawUI("Market Open", {infoX + 258.0f * ui, 31.0f}, 21.0f * ui, Theme::DOWN_GREEN);
    }

    DrawUI("Trader", {avatarButton.GetBounds().x - 78.0f, 34.0f}, 18.0f * ui, Theme::TEXT);
    avatarButton.Draw();
}

void TradingSystem::DrawMarketRow() {
    const float ui = GetUIScale();
    DrawRectangleRec(marketRow, Theme::PANEL);

    DrawLineEx(
        {0.0f, marketRow.y + marketRow.height - 1.0f},
        {marketRow.width, marketRow.y + marketRow.height - 1.0f},
        1.0f,
        Theme::BORDER_SOFT
    );

    const float margin = 40.0f;
    const float y = marketRow.y + 14.0f;
    const float rightInfoWidth = std::clamp(marketRow.width * 0.32f, 460.0f, 560.0f);
    const float metricsWidth = marketRow.width - margin * 2.0f - rightInfoWidth - 24.0f;
    const float gap = metricsWidth / 6.0f;
    const float x = margin;

    const StockData& stock = CurrentStock();

    DrawMarketItem("Price", FormatMoney(stock.price), x, y,
                   stock.changePercent >= 0 ? Theme::UP_RED : Theme::DOWN_GREEN);

    DrawMarketItem("Change", FormatChangePercent(stock.changePercent), x + gap, y,
                   stock.changePercent >= 0 ? Theme::UP_RED : Theme::DOWN_GREEN);

    DrawMarketItem("High", FormatMoney(stock.highPrice), x + gap * 2, y, Theme::UP_RED);
    DrawMarketItem("Low", FormatMoney(stock.lowPrice), x + gap * 3, y, Theme::DOWN_GREEN);
    DrawMarketItem("Open", FormatMoney(stock.openPrice), x + gap * 4, y, Theme::TEXT);
    DrawMarketItem("Volume", FormatIntWithComma(stock.volume), x + gap * 5, y, Theme::TEXT);

    float rightX = marketRow.width - margin - rightInfoWidth;
    float infoY = y;
    const double indexChange = GetMarketIndexChangePercent();
    DrawUI("Market Index", {rightX, infoY}, 17.0f * ui, Theme::MUTED);
    DrawNumber(FormatNumber(GetMarketIndex()), {rightX, infoY + 27.0f * ui}, 20.0f * ui, Theme::ACCENT_BLUE);

    DrawUI("Index Change", {rightX + 105.0f, infoY}, 15.0f * ui, Theme::MUTED);
    DrawNumber(FormatChangePercent(indexChange),
               {rightX + 105.0f, infoY + 27.0f * ui},
               15.0f * ui,
               indexChange >= 0.0 ? Theme::UP_RED : Theme::DOWN_GREEN);

    auto gainers = GetTopGainers(1);
    auto losers = GetTopLosers(1);

    DrawUI("Top Gainer (Market)", {rightX + 210.0f, infoY}, 14.0f * ui, Theme::MUTED);
    if (!gainers.empty()) {
        DrawUI(gainers.front().first + " " + FormatChangePercent(gainers.front().second),
               {rightX + 210.0f, infoY + 27.0f * ui},
               14.0f * ui,
               Theme::UP_RED);
    }

    DrawUI("Top Loser (Market)", {rightX + 330.0f, infoY}, 14.0f * ui, Theme::MUTED);
    if (!losers.empty()) {
        DrawUI(losers.front().first + " " + FormatChangePercent(losers.front().second),
               {rightX + 330.0f, infoY + 27.0f * ui},
               14.0f * ui,
               Theme::DOWN_GREEN);
    }
}

void TradingSystem::DrawMarketItem(
    const std::string& label,
    const std::string& value,
    float x,
    float y,
    Color valueColor
) {
    const float ui = GetUIScale();
    DrawUI(label, {x, y}, 16.0f * ui, Theme::MUTED);
    DrawNumber(value, {x, y + 29.0f * ui}, 25.0f * ui, valueColor);
}

void TradingSystem::DrawStockInfoPanel() {
    const float ui = GetUIScale();
    const StockData& stock = CurrentStock();
    DrawPanel(leftPanel);

    const float padX = 20.0f;
    const float titleY = leftPanel.y + 18.0f * ui;
    const float dropdownY = leftPanel.y + 76.0f * ui;
    const float dropdownHeight = std::min(50.0f, 42.0f * ui);

    DrawUI("Stock Info", {leftPanel.x + padX, titleY}, 27.0f * ui, Theme::TEXT);
    DrawUI("Select Stock", {leftPanel.x + padX, dropdownY - 20.0f * ui}, 14.0f * ui, Theme::MUTED);

    const int infoRowCount = 13;
    const float contentStartY = dropdownY + dropdownHeight + 22.0f * ui;
    const float contentBottomY = leftPanel.y + leftPanel.height - 26.0f * ui;
    const float rowGap = std::clamp(
        (contentBottomY - contentStartY) / static_cast<float>(infoRowCount - 1),
        26.0f * ui,
        34.0f * ui
    );
    float y = contentStartY;

    DrawInfoRow("Ticker", stock.ticker, y);
    y += rowGap;

    DrawInfoRow("Industry", stock.industry, y);
    y += rowGap;

    DrawInfoRow("Trend", TrendName(stock.trend), y, false, TrendColor(stock.trend));
    y += rowGap;

    DrawInfoRow(
        "Current Price",
        FormatMoney(stock.price),
        y,
        true,
        stock.changePercent >= 0 ? Theme::UP_RED : Theme::DOWN_GREEN
    );
    y += rowGap;

    DrawInfoRow(
        "Change",
        FormatChangePercent(stock.changePercent),
        y,
        true,
        stock.changePercent >= 0 ? Theme::UP_RED : Theme::DOWN_GREEN
    );
    y += rowGap;

    DrawInfoRow("Open", FormatMoney(stock.openPrice), y, true, Theme::TEXT, true);
    y += rowGap;

    DrawInfoRow("High", FormatMoney(stock.highPrice), y, true, Theme::UP_RED, true);
    y += rowGap;

    DrawInfoRow("Low", FormatMoney(stock.lowPrice), y, true, Theme::DOWN_GREEN, true);
    y += rowGap;

    DrawInfoRow("Limit Up Price", FormatMoney(GetLimitUpPrice(stock)), y, true, Theme::UP_RED, true);
    y += rowGap;

    DrawInfoRow("Limit Down Price", FormatMoney(GetLimitDownPrice(stock)), y, true, Theme::DOWN_GREEN, true);
    y += rowGap;

    DrawInfoRow("Current Status", GetLimitStatusName(stock), y, false, GetLimitStatusColor(stock), true);
    y += rowGap;

    DrawInfoRow("Volatility", FormatPercent(stock.volatility), y, true, Theme::TEXT, true);
    y += rowGap;

    DrawInfoRow("Momentum", FormatPercent(stock.momentum), y, true, Theme::TEXT, true);

    stockDropdown.Draw();
}

void TradingSystem::DrawChartPanel() {
    const float ui = GetUIScale();
    DrawPanel(chartPanel);
    DrawUI("Price Chart", {chartPanel.x + 20.0f, chartPanel.y + 18.0f}, 31.0f * ui, Theme::TEXT);

    const float newsHeight = std::clamp(chartPanel.height * 0.42f, 240.0f * ui, 330.0f * ui);
    const float chartHeight = std::max(120.0f, chartPanel.height - newsHeight - 112.0f);

    Rectangle chartArea = {
        chartPanel.x + 26.0f,
        chartPanel.y + 66.0f,
        chartPanel.width - 52.0f,
        chartHeight
    };

    DrawChart(chartArea);

    const float lowerGap = 14.0f;
    const float newsWidth = (chartPanel.width - 52.0f - lowerGap) * 0.74f;
    Rectangle newsArea = {
        chartPanel.x + 26.0f,
        chartPanel.y + chartPanel.height - newsHeight - 22.0f,
        newsWidth,
        newsHeight
    };

    Rectangle industryArea = {
        newsArea.x + newsArea.width + lowerGap,
        newsArea.y,
        chartPanel.width - 52.0f - newsWidth - lowerGap,
        newsHeight
    };

    DrawNewsPanel(newsArea);
    DrawIndustryPerformancePanel(industryArea);
}

void TradingSystem::DrawTradingPanel() {
    const float ui = GetUIScale();
    const StockData& stock = CurrentStock();
    DrawPanel(rightPanel);
    DrawUI("Trading Panel", {rightPanel.x + 20.0f, rightPanel.y + 20.0f}, 30.0f * ui, Theme::TEXT);

    float x = rightPanel.x + 25.0f;
    float col2 = x + (rightPanel.width - 50.0f) * 0.54f;

    double holdings = 0.0;
    double avgCost = 0.0;
    if (account.positions.count(stock.ticker)) {
        const Position& position = account.positions.at(stock.ticker);
        holdings = position.shares;
        avgCost = position.averageCost;
    }

    double positionValue = holdings * stock.price;
    double unrealizedPnL = holdings * (stock.price - avgCost);
    double totalAsset = TotalAsset();

    auto drawCompactValue = [](const std::string& label,
                               const std::string& value,
                               Vector2 pos,
                               bool numberValue,
                               Color valueColor) {
        const float ui = GetUIScale();
        DrawUI(label, pos, 16.0f * ui, Theme::MUTED);
        if (numberValue) {
            DrawNumber(value, {pos.x, pos.y + 26.0f * ui}, 22.0f * ui, valueColor);
        } else {
            DrawUI(value, {pos.x, pos.y + 26.0f * ui}, 21.0f * ui, valueColor);
        }
    };

    const float summaryTop = rightPanel.y + 58.0f * ui;
    const float orderTypeY = wholeButton.GetBounds().y - 28.0f * ui;
    const float summaryRowGap = std::clamp(
        (orderTypeY - summaryTop - 58.0f * ui) * 0.5f,
        38.0f * ui,
        54.0f * ui
    );

    drawCompactValue("Cash", FormatMoney(account.cash), {x, summaryTop}, true, Theme::TEXT);
    drawCompactValue(
        "Total Asset",
        FormatMoney(totalAsset),
        {col2, summaryTop},
        true,
        totalAsset >= account.initialCash ? Theme::UP_RED : Theme::DOWN_GREEN
    );

    drawCompactValue("Holding", FormatNumber(holdings) + " shares", {x, summaryTop + summaryRowGap}, false, Theme::TEXT);
    drawCompactValue("Avg Cost", avgCost > 0.0 ? FormatMoney(avgCost) : "-", {col2, summaryTop + summaryRowGap}, true, Theme::TEXT);

    drawCompactValue("Position Value", FormatMoney(positionValue), {x, summaryTop + summaryRowGap * 2.0f}, true, Theme::TEXT);
    drawCompactValue(
        "Unrealized P/L",
        FormatMoney(unrealizedPnL),
        {col2, summaryTop + summaryRowGap * 2.0f},
        true,
        unrealizedPnL >= 0.0 ? Theme::UP_RED : Theme::DOWN_GREEN
    );

    DrawUI("Order Type", {x, orderTypeY}, 16.0f * ui, Theme::MUTED);

    wholeButton.Draw();
    fractionalButton.Draw();

    DrawUI(
        tradeMode == TradeMode::WholeShare ? "Order Size (Board Lots)" : "Order Size (Odd-Lot Shares)",
        {x, quantityInputBox.y - 31.0f * ui},
        16.0f * ui,
        Theme::MUTED
    );

    minusButton.Draw();

    DrawRectangleRounded(
        quantityInputBox,
        0.10f,
        8,
        editingQuantity ? Theme::PANEL_3 : Theme::PANEL_2
    );

    DrawRectangleRoundedLines(
        quantityInputBox,
        0.10f,
        8,
        editingQuantity ? Theme::ACCENT_BLUE : Theme::BORDER
    );

    DrawUI(
        quantityInput,
        {quantityInputBox.x + 14.0f, quantityInputBox.y + 8.0f},
        21.0f * ui,
        Theme::TEXT
    );

    plusButton.Draw();

    DrawUI("Est. Cost", {x, quantityInputBox.y + 56.0f}, 16.0f * ui, Theme::MUTED);
    DrawNumber(FormatMoney(GetEstimatedCost()), {x + 105.0f * ui, quantityInputBox.y + 54.0f}, 20.0f * ui, Theme::TEXT);

    if (buyButton.GetBounds().y - quantityInputBox.y > 105.0f) {
        DrawUI("Actions", {x, buyButton.GetBounds().y - 28.0f * ui}, 16.0f * ui, Theme::MUTED);
    }

    buyButton.Draw();
    sellButton.Draw();
    nextDayButton.Draw();

    double estimatedCost = GetEstimatedCost();
    double orderShares = GetOrderShares();
    double held = 0.0;
    if (account.positions.count(stock.ticker)) {
        held = account.positions.at(stock.ticker).shares;
    }

    const float warningY = std::min(
        nextDayButton.GetBounds().y + 50.0f,
        rightPanel.y + rightPanel.height - 28.0f * ui
    );

    if (estimatedCost > account.cash) {
        DrawUI("Insufficient Cash", {x, warningY}, 15.0f * ui, Theme::UP_RED);
    }

    if (orderShares > held) {
        DrawUI("Insufficient Shares", {col2, warningY}, 15.0f * ui, Theme::UP_RED);
    }
}

void TradingSystem::DrawBottomSummary() {
    const float ui = GetUIScale();
    const StockData& stock = CurrentStock();
    DrawPanel(bottomPanel);

    float y = bottomPanel.y + 24.0f;
    const float x0 = bottomPanel.x + 25.0f;
    const float col = bottomPanel.width / 5.0f;

    DrawUI("Market Summary", {x0, y}, 23.0f * ui, Theme::TEXT);
    DrawUI("Open ", {x0 + col * 1.12f, y}, 19.0f * ui, Theme::TEXT);
    DrawNumber(FormatMoney(stock.openPrice), {x0 + col * 1.12f + 56.0f * ui, y}, 19.0f * ui, Theme::TEXT);
    DrawUI("High ", {x0 + col * 2.12f, y}, 19.0f * ui, Theme::TEXT);
    DrawNumber(FormatMoney(stock.highPrice), {x0 + col * 2.12f + 56.0f * ui, y}, 19.0f * ui, Theme::UP_RED);
    DrawUI("Low ", {x0 + col * 3.12f, y}, 19.0f * ui, Theme::TEXT);
    DrawNumber(FormatMoney(stock.lowPrice), {x0 + col * 3.12f + 44.0f * ui, y}, 19.0f * ui, Theme::DOWN_GREEN);
    DrawUI("Trend ", {x0 + col * 4.05f, y}, 19.0f * ui, Theme::TEXT);
    DrawUI(TrendName(stock.trend), {x0 + col * 4.05f + 66.0f * ui, y}, 19.0f * ui, TrendColor(stock.trend));
}

void TradingSystem::DrawPanel(Rectangle rect) {
    DrawRectangleRounded(rect, 0.04f, 12, Theme::PANEL);
    DrawRectangleRoundedLines(rect, 0.04f, 12, Theme::BORDER);
}

void TradingSystem::DrawInfoRow(
    const std::string& label,
    const std::string& value,
    float y,
    bool numberValue,
    Color color,
    bool compact
) {
    const float ui = GetUIScale();
    const float x = leftPanel.x + 24.0f;
    const float valueX = leftPanel.x + leftPanel.width * 0.58f;
    const float labelSize = (compact ? 13.0f : 14.5f) * ui;
    const float valueSize = (compact ? 16.0f : 18.0f) * ui;

    std::string displayLabel = label;
    if (leftPanel.width < 280.0f) {
        if (label == "Limit Up Price") {
            displayLabel = "Limit Up";
        } else if (label == "Limit Down Price") {
            displayLabel = "Limit Down";
        } else if (label == "Current Status") {
            displayLabel = "Status";
        }
    }

    DrawUI(displayLabel, {x, y}, labelSize, Theme::MUTED);
    if (numberValue) {
        DrawNumber(value, {valueX, y}, valueSize, color);
    } else {
        DrawUI(value, {valueX, y}, valueSize, color);
    }
}

void TradingSystem::DrawLabelValue(
    const std::string& label,
    const std::string& value,
    Vector2 pos,
    bool numberValue,
    Color valueColor
) {
    const float ui = GetUIScale();
    DrawUI(label, pos, 17.0f * ui, Theme::MUTED);
    if (numberValue) {
        DrawNumber(value, {pos.x, pos.y + 29.0f * ui}, 24.0f * ui, valueColor);
    } else {
        DrawUI(value, {pos.x, pos.y + 29.0f * ui}, 23.0f * ui, valueColor);
    }
}

void TradingSystem::DrawChart(Rectangle area) {
    const StockData& stock = CurrentStock();
    DrawRectangleRounded(area, 0.04f, 12, Theme::CHART_BG);
    DrawRectangleRoundedLines(area, 0.04f, 12, Theme::BORDER_SOFT);

    Rectangle grid = {
        area.x + 68.0f,
        area.y + 25.0f,
        area.width - 90.0f,
        area.height - 55.0f
    };

    for (int i = 0; i <= 4; ++i) {
        float y = grid.y + grid.height * i / 4.0f;
        DrawLineEx({grid.x, y}, {grid.x + grid.width, y}, 1.0f, Theme::GRID);
    }

    for (int i = 0; i <= 5; ++i) {
        float x = grid.x + grid.width * i / 5.0f;
        DrawLineEx({x, grid.y}, {x, grid.y + grid.height}, 1.0f, Theme::GRID);
    }

    if (stock.history.size() < 2) {
        return;
    }

    const double limitUp = GetLimitUpPrice(stock);
    const double limitDown = GetLimitDownPrice(stock);
    double minPrice = *std::min_element(stock.history.begin(), stock.history.end());
    double maxPrice = *std::max_element(stock.history.begin(), stock.history.end());

    if (std::abs(maxPrice - minPrice) < 1e-6) {
        maxPrice += 1.0;
        minPrice -= 1.0;
    }

    double padding = (maxPrice - minPrice) * 0.12;
    minPrice -= padding;
    maxPrice += padding;

    for (int i = 0; i <= 4; ++i) {
        double labelPrice = maxPrice - (maxPrice - minPrice) * i / 4.0;
        float y = grid.y + grid.height * i / 4.0f;

        DrawNumber(
            FormatMoney(labelPrice),
            {area.x + 15.0f, y - 9.0f},
            13.0f,
            Theme::MUTED
        );
    }

    auto priceToY = [&](double price) {
        return grid.y + grid.height -
               static_cast<float>((price - minPrice) / (maxPrice - minPrice)) * grid.height;
    };

    float dx = grid.width / static_cast<float>(stock.history.size() - 1);
    std::vector<Vector2> points;
    points.reserve(stock.history.size());

    for (size_t i = 0; i < stock.history.size(); ++i) {
        float x = grid.x + dx * static_cast<float>(i);
        float y = priceToY(stock.history[i]);

        points.push_back({x, y});
    }

    auto priceLimitStatus = [&](double price) {
        constexpr double epsilon = 1e-6;
        if (price >= limitUp - epsilon) {
            return LimitStatus::LimitUp;
        }
        if (price <= limitDown + epsilon) {
            return LimitStatus::LimitDown;
        }
        return LimitStatus::None;
    };

    for (size_t i = 1; i < points.size(); ++i) {
        const LimitStatus segmentLimitStatus = priceLimitStatus(stock.history[i]);
        Color lineColor = Theme::MUTED;

        if (segmentLimitStatus == LimitStatus::LimitUp) {
            lineColor = Theme::UP_RED;
        } else if (segmentLimitStatus == LimitStatus::LimitDown) {
            lineColor = Theme::DOWN_GREEN;
        } else if (stock.history[i] > stock.history[i - 1]) {
            lineColor = Theme::UP_RED;
        } else if (stock.history[i] < stock.history[i - 1]) {
            lineColor = Theme::DOWN_GREEN;
        }

        DrawLineEx(points[i - 1], points[i], 2.8f, lineColor);
    }

    Vector2 last = points.back();

    Color lastMarkerColor = GetLimitStatus(stock) == LimitStatus::None
        ? (stock.changePercent >= 0 ? Theme::UP_RED : Theme::DOWN_GREEN)
        : GetLimitStatusColor(stock);

    DrawCircleV(last, 4.8f, lastMarkerColor);

    DrawUI(
        stock.ticker,
        {grid.x + grid.width - 45.0f, grid.y + grid.height - 22.0f},
        14.0f,
        Theme::ACCENT_BLUE
    );

}

void TradingSystem::DrawNewsPanel(Rectangle area) {
    const float ui = GetUIScale();
    DrawRectangleRounded(area, 0.04f, 12, Theme::PANEL_2);
    DrawRectangleRoundedLines(area, 0.04f, 12, Theme::BORDER);

    const float pad = 16.0f;
    const float columnGap = 16.0f;
    const float leftX = area.x + pad;
    const float rightX = area.x + area.width - pad;
    const float availableWidth = area.width - pad * 2.0f;
    const float explanationWidth = std::clamp(area.width * 0.34f, 140.0f, 230.0f);
    const float leftColumnWidth = availableWidth - explanationWidth - columnGap;
    const float explanationX = rightX - explanationWidth;

    const float titleSize = std::min(30.0f, 24.0f * ui);
    const float headlineSize = std::min(27.0f, (area.height < 260.0f ? 18.0f : 21.0f) * ui);
    const float rowFontSize = std::min(21.0f, (area.height < 260.0f ? 14.0f : 16.0f) * ui);
    const float noteSize = std::min(15.0f, (area.height < 260.0f ? 11.0f : 12.0f) * ui);

    const float titleY = area.y + 12.0f;
    const float titleSeparatorY = titleY + titleSize + 8.0f;
    const float headlineY = titleSeparatorY + 14.0f * ui;
    const float contentStartY = headlineY + headlineSize + 18.0f * ui;
    const float labelX = leftX;
    const float valueX = leftX + std::clamp(leftColumnWidth * 0.48f, 96.0f, 132.0f);

    const int rowCount = 5;
    const float rowAreaBottom = area.y + area.height - 18.0f;
    const float rowSpacing = std::max(
        30.0f * ui,
        (rowAreaBottom - contentStartY) / static_cast<float>(rowCount)
    );

    if (activeNews.empty()) {
        DrawUI("Market News", {leftX, titleY}, titleSize, Theme::TEXT);
        DrawLineEx({leftX, titleSeparatorY}, {rightX, titleSeparatorY}, 1.0f, Theme::BORDER_SOFT);
        DrawUI("No active market events.", {leftX, headlineY}, 18.0f, Theme::MUTED);
        return;
    }

    const MarketNews& news = activeNews.front();

    DrawUI("Market News", {leftX, titleY}, titleSize, Theme::TEXT);
    DrawLineEx({leftX, titleSeparatorY}, {rightX, titleSeparatorY}, 1.0f, Theme::BORDER_SOFT);
    DrawUI(news.headline, {leftX, headlineY}, headlineSize, Theme::TEXT);

    float rowY = contentStartY;
    auto drawNewsRow = [&](const std::string& label,
                           const std::string& value,
                           Color valueColor) {
        DrawUI(label, {labelX, rowY}, rowFontSize, Theme::MUTED);
        DrawUI(value, {valueX, rowY}, rowFontSize, valueColor);
        rowY += rowSpacing;
    };

    drawNewsRow("Industry", news.industry, Theme::TEXT);
    drawNewsRow("Sentiment", SentimentName(news.sentiment), SentimentColor(news.sentiment));
    drawNewsRow("Trend Bias", TrendBiasLabel(news.trendBias), TrendBiasColor(news.trendBias));
    drawNewsRow("Volatility", NewsVolatilityLabel(news.volatilityMultiplier), NewsVolatilityColor(news.volatilityMultiplier));
    drawNewsRow("Days Remaining", std::to_string(news.daysRemaining), Theme::TEXT);

    Rectangle explanationBox = {
        explanationX,
        titleSeparatorY + 14.0f,
        explanationWidth,
        area.y + area.height - titleSeparatorY - 30.0f
    };

    DrawRectangleRounded(explanationBox, 0.04f, 8, Theme::PANEL_3);
    DrawRectangleRoundedLines(explanationBox, 0.04f, 8, Theme::BORDER_SOFT);

    const float boxPad = 12.0f * ui;
    float noteY = explanationBox.y + 12.0f * ui;
    DrawUI("How to read this", {explanationBox.x + boxPad, noteY}, noteSize + 2.0f, Theme::TEXT);
    noteY += 26.0f * ui;

    DrawUI("News changes", {explanationBox.x + boxPad, noteY}, noteSize, Theme::MUTED);
    noteY += 17.0f * ui;
    DrawUI("trend probability", {explanationBox.x + boxPad, noteY}, noteSize, Theme::MUTED);
    noteY += 17.0f * ui;
    DrawUI("and volatility,", {explanationBox.x + boxPad, noteY}, noteSize, Theme::MUTED);
    noteY += 17.0f * ui;
    DrawUI("not guaranteed", {explanationBox.x + boxPad, noteY}, noteSize, Theme::MUTED);
    noteY += 17.0f * ui;
    DrawUI("future returns.", {explanationBox.x + boxPad, noteY}, noteSize, Theme::MUTED);
}

void TradingSystem::DrawIndustryPerformancePanel(Rectangle area) {
    const float ui = GetUIScale();
    DrawRectangleRounded(area, 0.04f, 12, Theme::PANEL_2);
    DrawRectangleRoundedLines(area, 0.04f, 12, Theme::BORDER);

    const float pad = 16.0f;
    const float titleSize = std::min(25.0f, 21.0f * ui);
    DrawUI("Industry Performance", {area.x + pad, area.y + 14.0f}, titleSize, Theme::TEXT);

    auto performance = GetIndustryPerformance();
    float y = area.y + 54.0f * ui;
    const float rowGap = std::max(25.0f * ui, (area.height - 68.0f * ui) / 5.0f);
    const float labelX = area.x + pad;
    const float valueRightX = area.x + area.width - pad;

    for (const auto& item : performance) {
        if (y > area.y + area.height - 16.0f) {
            break;
        }

        std::string industryName = item.first;
        std::string valueText = FormatChangePercent(item.second);
        float labelSize = std::min(19.0f, 15.5f * ui);
        const float valueSize = std::min(20.0f, 16.5f * ui);
        const float valueWidth = TextWidth(valueText, valueSize, true);
        const float valueX = valueRightX - valueWidth;
        const float labelMaxWidth = valueX - labelX - 14.0f * ui;

        if (industryName == "Transportation" &&
            TextWidth(industryName, labelSize) > labelMaxWidth) {
            industryName = "Transport";
        }

        while (labelSize > 13.0f * ui &&
               TextWidth(industryName, labelSize) > labelMaxWidth) {
            labelSize -= 1.0f;
        }

        Color valueColor = item.second >= 0.0 ? Theme::UP_RED : Theme::DOWN_GREEN;
        DrawUI(industryName, {labelX, y}, labelSize, Theme::MUTED);
        DrawNumber(valueText, {valueX, y}, valueSize, valueColor);
        y += rowGap;
    }
}

StockData& TradingSystem::CurrentStock() {
    return stocks[selectedStock];
}

const StockData& TradingSystem::CurrentStock() const {
    return stocks[selectedStock];
}

void TradingSystem::GenerateNews() {
    std::vector<MarketNews> templates = {
        {"AI demand surges", "Technology", NewsSentiment::Positive, TrendType::Bullish, 1.4, 3},
        {"Bank regulations tighten", "Finance", NewsSentiment::Negative, TrendType::Bearish, 1.4, 4},
        {"Healthcare breakthrough", "Healthcare", NewsSentiment::Positive, TrendType::Bullish, 1.3, 3},
        {"Oil prices collapse", "Energy", NewsSentiment::Negative, TrendType::Bearish, 1.4, 4},
        {"Global shipping boom", "Transportation", NewsSentiment::Positive, TrendType::Bullish, 1.5, 3},
        {"Chip export restrictions ease", "Technology", NewsSentiment::Positive, TrendType::Bullish, 1.3, 3},
        {"Loan defaults rise", "Finance", NewsSentiment::Negative, TrendType::Bearish, 1.6, 3},
        {"Hospital demand stabilizes", "Healthcare", NewsSentiment::Neutral, TrendType::Neutral, 1.2, 2},
        {"Refinery outage disrupts supply", "Energy", NewsSentiment::Negative, TrendType::Bearish, 1.6, 3},
        {"Port congestion clears", "Transportation", NewsSentiment::Positive, TrendType::Bullish, 1.4, 3}
    };

    std::uniform_int_distribution<int> dist(0, static_cast<int>(templates.size()) - 1);
    MarketNews news = templates[dist(rng)];
    activeNews.clear();
    activeNews.push_back(news);
    newsHistory.emplace_back(day, news);
}

double TradingSystem::TotalAsset() const {
    return account.cash + GetPortfolioValue();
}

double TradingSystem::GetOrderShares() const {
    if (tradeMode == TradeMode::WholeShare) {
        return orderQuantity * SHARES_PER_LOT;
    }

    return orderQuantity;
}

double TradingSystem::GetEstimatedCost() const {
    return GetOrderShares() * CurrentStock().price;
}

double TradingSystem::GetPortfolioValue() const {
    double value = 0.0;
    for (const auto& item : account.positions) {
        const std::string& ticker = item.first;
        const Position& position = item.second;
        for (const auto& stock : stocks) {
            if (stock.ticker == ticker) {
                value += position.shares * stock.price;
                break;
            }
        }
    }
    return value;
}

double TradingSystem::GetMarketIndex() const {
    return marketIndex;
}

double TradingSystem::GetMarketIndexChangePercent() const {
    if (marketOpenIndex <= 0.0) {
        return 0.0;
    }

    return ((marketIndex - marketOpenIndex) / marketOpenIndex) * 100.0;
}

double TradingSystem::RandomDouble(double low, double high) {
    std::uniform_real_distribution<double> dist(low, high);
    return dist(rng);
}

double TradingSystem::RandomNormal() {
    static std::normal_distribution<double> dist(0.0, 1.0);
    return dist(rng);
}

double TradingSystem::GetNewsInfluenceForStock(const StockData& stock) const {
    if (activeNews.empty() || activeNews.front().daysRemaining <= 0) {
        return 0.0;
    }

    return stock.industry == activeNews.front().industry ? 1.0 : 0.2;
}

double TradingSystem::GetTrendBiasDirection(const StockData& stock) const {
    if (activeNews.empty() || activeNews.front().daysRemaining <= 0) {
        return 0.0;
    }

    if (activeNews.front().trendBias == TrendType::Bullish) {
        return 1.0;
    }

    if (activeNews.front().trendBias == TrendType::Bearish) {
        return -1.0;
    }

    return 0.0;
}

double TradingSystem::GetLimitUpPrice(const StockData& stock) const {
    return stock.openPrice * 1.10;
}

double TradingSystem::GetLimitDownPrice(const StockData& stock) const {
    return stock.openPrice * 0.90;
}

LimitStatus TradingSystem::GetLimitStatus(const StockData& stock) const {
    constexpr double epsilon = 1e-6;

    if (stock.limitStatus != LimitStatus::None) {
        return stock.limitStatus;
    }

    if (stock.price >= GetLimitUpPrice(stock) - epsilon) {
        return LimitStatus::LimitUp;
    }

    if (stock.price <= GetLimitDownPrice(stock) + epsilon) {
        return LimitStatus::LimitDown;
    }

    return LimitStatus::None;
}

std::string TradingSystem::GetLimitStatusName(const StockData& stock) const {
    switch (GetLimitStatus(stock)) {
        case LimitStatus::LimitUp:
            return "Limit Up";
        case LimitStatus::LimitDown:
            return "Limit Down";
        default:
            return "Normal";
    }
}

Color TradingSystem::GetLimitStatusColor(const StockData& stock) const {
    switch (GetLimitStatus(stock)) {
        case LimitStatus::LimitUp:
            return Theme::UP_RED;
        case LimitStatus::LimitDown:
            return Theme::DOWN_GREEN;
        default:
            return Theme::TEXT;
    }
}

std::vector<std::pair<std::string, double>> TradingSystem::GetTopGainers(int count) const {
    std::vector<std::pair<std::string, double>> candidates;
    for (const auto& stock : stocks) {
        if (stock.changePercent >= 0.0) {
            candidates.emplace_back(stock.ticker, stock.changePercent);
        }
    }
    std::sort(candidates.begin(), candidates.end(), [](auto& a, auto& b) {
        return a.second > b.second;
    });
    if (static_cast<int>(candidates.size()) > count) {
        candidates.resize(count);
    }
    return candidates;
}

std::vector<std::pair<std::string, double>> TradingSystem::GetTopLosers(int count) const {
    std::vector<std::pair<std::string, double>> candidates;
    for (const auto& stock : stocks) {
        if (stock.changePercent < 0.0) {
            candidates.emplace_back(stock.ticker, stock.changePercent);
        }
    }
    std::sort(candidates.begin(), candidates.end(), [](auto& a, auto& b) {
        return a.second < b.second;
    });
    if (static_cast<int>(candidates.size()) > count) {
        candidates.resize(count);
    }
    return candidates;
}

std::vector<std::pair<std::string, double>> TradingSystem::GetIndustryPerformance() const {
    std::vector<std::string> industries = {
        "Technology",
        "Finance",
        "Healthcare",
        "Energy",
        "Transportation"
    };

    std::vector<std::pair<std::string, double>> performance;
    performance.reserve(industries.size());

    for (const auto& industry : industries) {
        double totalChange = 0.0;
        int count = 0;

        for (const auto& stock : stocks) {
            if (stock.industry == industry) {
                totalChange += stock.changePercent;
                count++;
            }
        }

        if (count > 0) {
            performance.emplace_back(industry, totalChange / static_cast<double>(count));
        }
    }

    return performance;
}

std::string TradingSystem::TimeString() const {
    std::ostringstream ss;

    ss << std::setw(2) << std::setfill('0') << hour << ":"
       << std::setw(2) << std::setfill('0') << minute << ":"
       << std::setw(2) << std::setfill('0') << second;

    return ss.str();
}

int TradingSystem::RandomInt(int low, int high) {
    std::uniform_int_distribution<int> dist(low, high);
    return dist(rng);
}
