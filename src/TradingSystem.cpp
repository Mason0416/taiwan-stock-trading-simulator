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
#include <vector>

TradingSystem::TradingSystem()
    : rng(std::random_device{}()) {
    InitWindow(screenWidth, screenHeight, "Raylib Stock Trading");
    SetTargetFPS(60);

    SetupFont();
    InitStock();
    PickTrendForNewDay();
    SeedInitialHistory();
    SetupUI();
}

TradingSystem::~TradingSystem() {
    UnloadFontSafe();
    CloseWindow();
}

void TradingSystem::SeedInitialHistory() {
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

        price = std::max(1.0, price + drift + seedNoise(rng));
        stock.history.push_back(price);
    }

    stock.price = stock.history.back();
    stock.previousPrice = stock.history.size() >= 2
        ? stock.history[stock.history.size() - 2]
        : stock.price;
    stock.highPrice = *std::max_element(stock.history.begin(), stock.history.end());
    stock.lowPrice = *std::min_element(stock.history.begin(), stock.history.end());
    stock.changePercent = ((stock.price - stock.openPrice) / stock.openPrice) * 100.0;
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

void TradingSystem::InitStock() {
    stock.name = "Aurora Tech";
    stock.ticker = "AUR";
    stock.industry = "Technology";

    stock.price = 120.0;
    stock.previousPrice = 120.0;
    stock.openPrice = 120.0;
    stock.highPrice = 120.0;
    stock.lowPrice = 120.0;

    stock.changePercent = 0.0;
    stock.volume = 806884;
    stock.volatility = 0.0012;
    stock.momentum = 0.0;

    stock.history.clear();
    stock.history.push_back(stock.price);
}

void TradingSystem::SetupUI() {
    stockDropdown = StockDropdown({60.0f, 250.0f, 220.0f, 44.0f});

    wholeButton = Button(
        {1065.0f, 455.0f, 130.0f, 42.0f},
        "Whole",
        Theme::GRAY_BUTTON,
        Theme::GRAY_HOVER,
        Theme::TEXT
    );

    fractionalButton = Button(
        {1205.0f, 455.0f, 130.0f, 42.0f},
        "Fractional",
        Theme::GRAY_BUTTON,
        Theme::GRAY_HOVER,
        Theme::TEXT
    );

    minusButton = Button(
        {1065.0f, 525.0f, 45.0f, 42.0f},
        "-",
        Theme::GRAY_BUTTON,
        Theme::GRAY_HOVER,
        Theme::TEXT
    );

    plusButton = Button(
        {1290.0f, 525.0f, 45.0f, 42.0f},
        "+",
        Theme::GRAY_BUTTON,
        Theme::GRAY_HOVER,
        Theme::TEXT
    );

    buyButton = Button(
        {1065.0f, 605.0f, 130.0f, 42.0f},
        "Buy",
        Theme::UP_RED,
        Color{230, 86, 95, 255},
        WHITE
    );

    sellButton = Button(
        {1205.0f, 605.0f, 130.0f, 42.0f},
        "Sell",
        Theme::DOWN_GREEN,
        Color{26, 184, 120, 255},
        WHITE
    );

    nextDayButton = Button(
        {1065.0f, 655.0f, 270.0f, 42.0f},
        "Next Day",
        Theme::GRAY_BUTTON,
        Theme::GRAY_HOVER,
        Theme::TEXT
    );

    avatarButton = Button(
        {1305.0f, 20.0f, 56.0f, 56.0f},
        "A",
        Theme::ACCENT_BLUE,
        Theme::ACCENT_BLUE_HOVER,
        WHITE
    );

    backButton = Button(
        {80.0f, 28.0f, 110.0f, 42.0f},
        "Back",
        Theme::GRAY_BUTTON,
        Theme::GRAY_HOVER,
        Theme::TEXT
    );

    historyButton = Button(
        {210.0f, 28.0f, 145.0f, 42.0f},
        "History",
        Theme::GRAY_BUTTON,
        Theme::GRAY_HOVER,
        Theme::TEXT
    );

    UpdateTradeModeButtons();
}

void TradingSystem::Update(float dt) {
    if (page == Page::Main) {
        UpdateMain(dt);
    } else if (page == Page::Account) {
        UpdateAccount(dt);
    } else {
        UpdateHistory(dt);
    }
}

void TradingSystem::UpdateMain(float dt) {
    stockDropdown.Update();

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
        quantityInput = FormatNumber(orderQuantity);
        UpdateTradeModeButtons();
    }

    if (fractionalButton.IsClicked()) {
        tradeMode = TradeMode::FractionalShare;
        if (orderQuantity < 0.25) {
            orderQuantity = 0.25;
        }
        quantityInput = FormatNumber(orderQuantity);
        UpdateTradeModeButtons();
    }

    if (plusButton.IsClicked()) {
        double step = (tradeMode == TradeMode::WholeShare) ? 1.0 : 0.25;
        orderQuantity += step;
        quantityInput = FormatNumber(orderQuantity);
    }

    if (minusButton.IsClicked()) {
        double step = (tradeMode == TradeMode::WholeShare) ? 1.0 : 0.25;
        orderQuantity = std::max(step, orderQuantity - step);
        quantityInput = FormatNumber(orderQuantity);
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        editingQuantity = CheckCollisionPointRec(GetMousePosition(), quantityInputBox);
    }

    if (editingQuantity) {
        int key = GetCharPressed();

        while (key > 0) {
            char c = static_cast<char>(key);

            if ((c >= '0' && c <= '9') || c == '.') {
                if (!(c == '.' && quantityInput.find('.') != std::string::npos)) {
                    quantityInput.push_back(c);
                }
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
        double value = std::stod(quantityInput);

        if (tradeMode == TradeMode::WholeShare) {
            value = std::max(1.0, std::round(value));
        } else {
            value = std::max(0.25, value);
        }

        orderQuantity = value;
        quantityInput = FormatNumber(orderQuantity);
    } catch (...) {
        quantityInput = FormatNumber(orderQuantity);
    }
}

void TradingSystem::UpdateAccount(float dt) {
    backButton.Update(dt);
    historyButton.Update(dt);
    avatarButton.Update(dt);

    if (backButton.IsClicked()) {
        page = Page::Main;
    }

    if (historyButton.IsClicked()) {
        page = Page::History;
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
    fractionalButton.SetActive(tradeMode == TradeMode::FractionalShare);
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

    stock.previousPrice = stock.price;

    double drift = 0.0;

    if (stock.trend == TrendType::Bullish) {
        drift = stock.price * 0.0035;
    } else if (stock.trend == TrendType::Bearish) {
        drift = -stock.price * 0.0035;
    }

    double noiseStrength = (stock.trend == TrendType::Neutral)
        ? stock.price * 0.0020
        : stock.price * 0.0012;

    std::normal_distribution<double> noise(0.0, noiseStrength);
    double nextPrice = stock.price + drift + noise(rng);

    stock.price = std::max(1.0, nextPrice);
    stock.highPrice = std::max(stock.highPrice, stock.price);
    stock.lowPrice = std::min(stock.lowPrice, stock.price);
    stock.changePercent = ((stock.price - stock.openPrice) / stock.openPrice) * 100.0;
    stock.momentum = (stock.price - stock.openPrice) / stock.openPrice * 100.0;
    stock.volume += RandomInt(1800, 5200);

    stock.history.push_back(stock.price);

    if (stock.history.size() > 90) {
        stock.history.erase(stock.history.begin());
    }
}

void TradingSystem::PickTrendForNewDay() {
    std::uniform_int_distribution<int> trendDist(0, 2);
    int choice = trendDist(rng);

    if (choice == 0) {
        stock.trend = TrendType::Bullish;
    } else if (choice == 1) {
        stock.trend = TrendType::Neutral;
    } else {
        stock.trend = TrendType::Bearish;
    }

    stock.volatility = (stock.trend == TrendType::Neutral) ? 0.20 : 0.12;
}

void TradingSystem::NextDay() {
    day++;
    hour = 9;
    minute = 0;
    second = 0;
    timeAccumulator = 0.0f;
    priceAccumulator = 0.0f;

    PickTrendForNewDay();

    stock.openPrice = stock.price;
    stock.previousPrice = stock.price;
    stock.highPrice = stock.price;
    stock.lowPrice = stock.price;
    stock.changePercent = 0.0;
    stock.momentum = 0.0;

    stock.history.clear();

    for (int i = 0; i < 40; ++i) {
        stock.history.push_back(stock.price);
    }
}

void TradingSystem::Buy() {
    double cost = GetEstimatedCost();

    if (account.cash < cost) {
        return;
    }

    account.cash -= cost;
    account.shares += GetOrderShares();

    account.trades.push_back({
        true,
        stock.price,
        GetOrderShares(),
        TimeString()
    });
}

void TradingSystem::Sell() {
    double qty = GetOrderShares();

    if (account.shares >= qty) {
        account.cash += qty * stock.price;
        account.shares -= qty;
        account.trades.push_back({
            false,
            stock.price,
            qty,
            TimeString()
        });
    }
}

void TradingSystem::Draw() {
    if (page == Page::Main) {
        DrawMain();
    } else if (page == Page::Account) {
        DrawAccount();
    } else {
        DrawHistory();
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

    Rectangle panel = {60.0f, 135.0f, 1280.0f, 620.0f};
    DrawPanel(panel);

    DrawUI("Account Info", {90.0f, 165.0f}, 24.0f, Theme::TEXT);

    double total = TotalAsset();
    double pnl = total - account.initialCash;
    double pnlPercent = pnl / account.initialCash * 100.0;

    DrawLabelValue("Player", "Trader", {90.0f, 220.0f});
    DrawLabelValue("Initial Cash", FormatMoney(account.initialCash), {90.0f, 270.0f});
    DrawLabelValue("Cash", FormatMoney(account.cash), {90.0f, 320.0f});
    DrawLabelValue("Shares", FormatNumber(account.shares), {90.0f, 370.0f});
    DrawLabelValue("Current Price", FormatMoney(stock.price), {90.0f, 420.0f});

    DrawLabelValue(
        "Total Asset",
        FormatMoney(total),
        {90.0f, 470.0f},
        pnl >= 0.0 ? Theme::UP_RED : Theme::DOWN_GREEN
    );

    DrawLabelValue(
        "Total Return",
        FormatMoney(pnl) + " (" + FormatChangePercent(pnlPercent) + ")",
        {90.0f, 520.0f},
        pnl >= 0.0 ? Theme::UP_RED : Theme::DOWN_GREEN
    );

    backButton.Draw();
    historyButton.Draw();
    avatarButton.Draw();
}

void TradingSystem::DrawHistory() {
    DrawHeader("", false);

    Rectangle panel = {60.0f, 135.0f, 1280.0f, 620.0f};
    DrawPanel(panel);

    DrawUI("Trade Records", {90.0f, 165.0f}, 24.0f, Theme::TEXT);

    if (account.trades.empty()) {
        DrawUI("No trade history yet.", {90.0f, 220.0f}, 18.0f, Theme::MUTED);
    } else {
        float y = 220.0f;

        for (int i = static_cast<int>(account.trades.size()) - 1; i >= 0 && y < 725.0f; --i) {
            const TradeRecord& t = account.trades[i];

            std::ostringstream ss;
            ss << (t.isBuy ? "BUY  " : "SELL ")
               << std::fixed << std::setprecision(2)
               << t.quantity << " share(s) at "
               << FormatMoney(t.price)
               << "    Time: " << t.time;

            DrawUI(
                ss.str(),
                {90.0f, y},
                20.0f,
                t.isBuy ? Theme::UP_RED : Theme::DOWN_GREEN
            );

            y += 34.0f;
        }
    }

    backButton.Draw();
    avatarButton.Draw();
}

void TradingSystem::DrawHeader(const std::string& title, bool drawInfo) {
    DrawRectangleRec(header, Theme::TOP);

    DrawLineEx(
        {0.0f, header.height - 1.0f},
        {static_cast<float>(screenWidth), header.height - 1.0f},
        1.5f,
        Theme::BORDER_SOFT
    );

    DrawUI(title, {40.0f, 30.0f}, 34.0f, Theme::TEXT);

    if (drawInfo) {
        DrawUI("Day " + std::to_string(day), {570.0f, 31.0f}, 20.0f, Theme::MUTED);
        DrawUI(TimeString(), {670.0f, 31.0f}, 20.0f, Theme::TEXT);
        DrawUI("Market Open", {805.0f, 31.0f}, 20.0f, Theme::DOWN_GREEN);
    }

    DrawUI("Trader", {1235.0f, 34.0f}, 16.0f, Theme::TEXT);
    avatarButton.Draw();
}

void TradingSystem::DrawMarketRow() {
    DrawRectangleRec(marketRow, Theme::PANEL);

    DrawLineEx(
        {0.0f, marketRow.y + marketRow.height - 1.0f},
        {static_cast<float>(screenWidth), marketRow.y + marketRow.height - 1.0f},
        1.0f,
        Theme::BORDER_SOFT
    );

    float x = 40.0f;
    float y = marketRow.y + 18.0f;
    float gap = 215.0f;

    DrawMarketItem("Price", FormatMoney(stock.price), x, y,
                   stock.changePercent >= 0 ? Theme::UP_RED : Theme::DOWN_GREEN);

    DrawMarketItem("Change", FormatChangePercent(stock.changePercent), x + gap, y,
                   stock.changePercent >= 0 ? Theme::UP_RED : Theme::DOWN_GREEN);

    DrawMarketItem("High", FormatMoney(stock.highPrice), x + gap * 2, y, Theme::UP_RED);
    DrawMarketItem("Low", FormatMoney(stock.lowPrice), x + gap * 3, y, Theme::DOWN_GREEN);
    DrawMarketItem("Open", FormatMoney(stock.openPrice), x + gap * 4, y, Theme::TEXT);
    DrawMarketItem("Volume", FormatIntWithComma(stock.volume), x + gap * 5, y, Theme::TEXT);
}

void TradingSystem::DrawMarketItem(
    const std::string& label,
    const std::string& value,
    float x,
    float y,
    Color valueColor
) {
    DrawUI(label, {x, y}, 16.0f, Theme::MUTED);
    DrawUI(value, {x, y + 28.0f}, 26.0f, valueColor);
}

void TradingSystem::DrawStockInfoPanel() {
    DrawPanel(leftPanel);

    DrawUI("Stock Info", {leftPanel.x + 20.0f, leftPanel.y + 20.0f}, 26.0f, Theme::TEXT);
    DrawUI("Select Stock", {leftPanel.x + 20.0f, leftPanel.y + 58.0f}, 14.0f, Theme::MUTED);

    float y = 305.0f;

    DrawInfoRow("Ticker", stock.ticker, y);
    y += 39.0f;

    DrawInfoRow("Industry", stock.industry, y);
    y += 39.0f;

    DrawInfoRow("Trend", TrendName(stock.trend), y, TrendColor(stock.trend));
    y += 39.0f;

    DrawInfoRow(
        "Current Price",
        FormatMoney(stock.price),
        y,
        stock.changePercent >= 0 ? Theme::UP_RED : Theme::DOWN_GREEN
    );
    y += 39.0f;

    DrawInfoRow(
        "Change",
        FormatChangePercent(stock.changePercent),
        y,
        stock.changePercent >= 0 ? Theme::UP_RED : Theme::DOWN_GREEN
    );
    y += 39.0f;

    DrawInfoRow("Open", FormatMoney(stock.openPrice), y);
    y += 39.0f;

    DrawInfoRow("High", FormatMoney(stock.highPrice), y, Theme::UP_RED);
    y += 39.0f;

    DrawInfoRow("Low", FormatMoney(stock.lowPrice), y, Theme::DOWN_GREEN);
    y += 39.0f;

    DrawInfoRow("Volatility", FormatPercent(stock.volatility), y);
    y += 39.0f;

    DrawInfoRow("Momentum", FormatPercent(stock.momentum), y);

    stockDropdown.Draw();
}

void TradingSystem::DrawChartPanel() {
    DrawPanel(chartPanel);
    DrawUI("Price Chart", {chartPanel.x + 20.0f, chartPanel.y + 20.0f}, 28.0f, Theme::TEXT);

    Rectangle chartArea = {
        chartPanel.x + 26.0f,
        chartPanel.y + 62.0f,
        chartPanel.width - 52.0f,
        chartPanel.height - 88.0f
    };

    DrawChart(chartArea);
}

void TradingSystem::DrawTradingPanel() {
    DrawPanel(rightPanel);
    DrawUI("Trading Panel", {rightPanel.x + 20.0f, rightPanel.y + 20.0f}, 28.0f, Theme::TEXT);

    float x = rightPanel.x + 25.0f;

    DrawLabelValue("Cash", FormatMoney(account.cash), {x, 265.0f});
    DrawLabelValue("Shares", FormatNumber(account.shares), {x, 325.0f});

    DrawLabelValue(
        "Total Asset",
        FormatMoney(TotalAsset()),
        {x, 385.0f},
        TotalAsset() >= account.initialCash ? Theme::UP_RED : Theme::DOWN_GREEN
    );

    DrawUI("Order Type", {x, 430.0f}, 15.0f, Theme::MUTED);

    wholeButton.Draw();
    fractionalButton.Draw();

    DrawUI(
        tradeMode == TradeMode::WholeShare ? "Order Size (Lots)" : "Order Size (Shares)",
        {x, 505.0f},
        15.0f,
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
        {quantityInputBox.x + 14.0f, quantityInputBox.y + 10.0f},
        18.0f,
        Theme::TEXT
    );

    plusButton.Draw();

    std::string unitText = (tradeMode == TradeMode::WholeShare) ? "Lots" : "Shares";

    DrawUI(
        unitText,
        {quantityInputBox.x, quantityInputBox.y + 48.0f},
        14.0f,
        Theme::MUTED
    );

    DrawUI("Est. Cost", {x, 585.0f}, 14.0f, Theme::MUTED);
    DrawUI(FormatMoney(GetEstimatedCost()), {x + 95.0f, 585.0f}, 16.0f, Theme::TEXT);

    DrawUI("Actions", {x, 620.0f}, 15.0f, Theme::MUTED);

    buyButton.Draw();
    sellButton.Draw();
    nextDayButton.Draw();

    double estimatedCost = GetEstimatedCost();
    double orderShares = GetOrderShares();

    if (estimatedCost > account.cash) {
        DrawUI("Insufficient Cash", {x, 710.0f}, 16.0f, Theme::UP_RED);
    }

    if (orderShares > account.shares) {
        DrawUI("Insufficient Shares", {x, 735.0f}, 16.0f, Theme::DOWN_GREEN);
    }
}

void TradingSystem::DrawBottomSummary() {
    DrawPanel(bottomPanel);

    float y = bottomPanel.y + 27.0f;

    DrawUI("Market Summary", {bottomPanel.x + 25.0f, y}, 22.0f, Theme::TEXT);
    DrawUI("Open " + FormatMoney(stock.openPrice), {bottomPanel.x + 310.0f, y}, 18.0f, Theme::TEXT);
    DrawUI("High " + FormatMoney(stock.highPrice), {bottomPanel.x + 560.0f, y}, 18.0f, Theme::UP_RED);
    DrawUI("Low " + FormatMoney(stock.lowPrice), {bottomPanel.x + 805.0f, y}, 18.0f, Theme::DOWN_GREEN);
    DrawUI("Trend " + TrendName(stock.trend), {bottomPanel.x + 1040.0f, y}, 18.0f, TrendColor(stock.trend));
}

void TradingSystem::DrawPanel(Rectangle rect) {
    DrawRectangleRounded(rect, 0.04f, 12, Theme::PANEL);
    DrawRectangleRoundedLines(rect, 0.04f, 12, Theme::BORDER);
}

void TradingSystem::DrawInfoRow(
    const std::string& label,
    const std::string& value,
    float y,
    Color color
) {
    float x = leftPanel.x + 28.0f;

    DrawUI(label, {x, y}, 14.0f, Theme::MUTED);
    DrawUI(value, {x, y + 20.0f}, 18.0f, color);
}

void TradingSystem::DrawLabelValue(
    const std::string& label,
    const std::string& value,
    Vector2 pos,
    Color valueColor
) {
    DrawUI(label, pos, 16.0f, Theme::MUTED);
    DrawUI(value, {pos.x, pos.y + 26.0f}, 22.0f, valueColor);
}

void TradingSystem::DrawChart(Rectangle area) {
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

        DrawUI(
            FormatMoney(labelPrice),
            {area.x + 15.0f, y - 9.0f},
            13.0f,
            Theme::MUTED
        );
    }

    float dx = grid.width / static_cast<float>(stock.history.size() - 1);
    std::vector<Vector2> points;
    points.reserve(stock.history.size());

    for (size_t i = 0; i < stock.history.size(); ++i) {
        float x = grid.x + dx * static_cast<float>(i);
        float y = grid.y + grid.height -
                  static_cast<float>((stock.history[i] - minPrice) / (maxPrice - minPrice)) * grid.height;

        points.push_back({x, y});
    }

    for (size_t i = 1; i < points.size(); ++i) {
        Color lineColor = stock.history[i] >= stock.history[i - 1]
            ? Theme::UP_RED
            : Theme::DOWN_GREEN;

        DrawLineEx(points[i - 1], points[i], 2.8f, lineColor);
    }

    Vector2 last = points.back();

    DrawCircleV(
        last,
        4.8f,
        stock.changePercent >= 0 ? Theme::UP_RED : Theme::DOWN_GREEN
    );

    DrawUI(
        stock.ticker,
        {grid.x + grid.width - 45.0f, grid.y + grid.height - 22.0f},
        14.0f,
        Theme::ACCENT_BLUE
    );
}

double TradingSystem::TotalAsset() const {
    return account.cash + account.shares * stock.price;
}

double TradingSystem::GetOrderShares() const {
    if (tradeMode == TradeMode::WholeShare) {
        return orderQuantity * SHARES_PER_LOT;
    }

    return orderQuantity;
}

double TradingSystem::GetEstimatedCost() const {
    return GetOrderShares() * stock.price;
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