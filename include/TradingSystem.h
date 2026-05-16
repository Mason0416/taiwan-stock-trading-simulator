// TradingSystem.h
#pragma once

#include "raylib.h"

#include "StockData.h"
#include "Account.h"
#include "GUI.h"

#include <random>
#include <string>

enum class TradeMode {
    WholeShare,
    FractionalShare
};

enum class Page {
    Main,
    Account,
    History
};

class TradingSystem {
public:
    TradingSystem();

    ~TradingSystem();

    void Run();

private:

    static constexpr int screenWidth = 1400;
    static constexpr int screenHeight = 900;
    static constexpr double SHARES_PER_LOT = 1000.0;

    // =========================
    // Data
    // =========================

    StockData stock;

    Account account;

    TradeMode tradeMode = TradeMode::WholeShare;

    Page page = Page::Main;

    // =========================
    // Order
    // =========================

    double orderQuantity = 1.0;

    std::string quantityInput = "1.00";

    bool editingQuantity = false;

    Rectangle quantityInputBox = {
        1125.0f,
        525.0f,
        150.0f,
        42.0f
    };

    // =========================
    // Time
    // =========================

    int day = 1;

    int hour = 9;

    int minute = 0;

    int second = 0;

    float timeAccumulator = 0.0f;

    float priceAccumulator = 0.0f;

    // =========================
    // Random
    // =========================

    std::mt19937 rng;

    // =========================
    // Layout
    // =========================

    Rectangle header = {
        0.0f,
        0.0f,
        1400.0f,
        95.0f
    };

    Rectangle marketRow = {
        0.0f,
        95.0f,
        1400.0f,
        75.0f
    };

    Rectangle leftPanel = {
        40.0f,
        190.0f,
        260.0f,
        570.0f
    };

    Rectangle chartPanel = {
        320.0f,
        190.0f,
        700.0f,
        570.0f
    };

    Rectangle rightPanel = {
        1040.0f,
        190.0f,
        320.0f,
        570.0f
    };

    Rectangle bottomPanel = {
        40.0f,
        785.0f,
        1320.0f,
        75.0f
    };

    // =========================
    // GUI
    // =========================

    StockDropdown stockDropdown;

    Button wholeButton;

    Button fractionalButton;

    Button buyButton;

    Button sellButton;

    Button nextDayButton;

    Button minusButton;

    Button plusButton;

    Button avatarButton;

    Button backButton;

    Button historyButton;

    // =========================
    // Setup
    // =========================

    void InitStock();

    void SetupUI();

    void SeedInitialHistory();

    // =========================
    // Update
    // =========================

    void Update(float dt);

    void UpdateMain(float dt);

    void UpdateAccount(float dt);

    void UpdateHistory(float dt);

    void ApplyQuantityInput();

    void UpdateTradeModeButtons();

    // =========================
    // Market Logic
    // =========================

    void AdvanceTime(float dt);

    void UpdatePrice(float dt);

    void PickTrendForNewDay();

    void NextDay();

    // =========================
    // Trading
    // =========================

    void Buy();

    void Sell();

    // =========================
    // Draw
    // =========================

    void Draw();

    void DrawMain();

    void DrawAccount();

    void DrawHistory();

    void DrawHeader(
        const std::string& title,
        bool drawInfo
    );

    void DrawMarketRow();

    void DrawMarketItem(
        const std::string& label,
        const std::string& value,
        float x,
        float y,
        Color valueColor
    );

    void DrawStockInfoPanel();

    void DrawChartPanel();

    void DrawTradingPanel();

    void DrawBottomSummary();

    void DrawPanel(Rectangle rect);

    void DrawInfoRow(
        const std::string& label,
        const std::string& value,
        float y,
        Color color = Theme::TEXT
    );

    void DrawLabelValue(
        const std::string& label,
        const std::string& value,
        Vector2 pos,
        Color valueColor = Theme::TEXT
    );

    void DrawChart(Rectangle area);

    // =========================
    // Helpers
    // =========================

    double TotalAsset() const;

    double GetOrderShares() const;

    double GetEstimatedCost() const;

    std::string TimeString() const;

    int RandomInt(int low, int high);
};