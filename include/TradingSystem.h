// TradingSystem.h
#pragma once

#include "raylib.h"

#include "StockData.h"
#include "Account.h"
#include "StockDropdown.h"
#include "GUI.h"
#include "NewsEvent.h"

#include <random>
#include <string>
#include <vector>

enum class TradeMode {
    WholeShare,
    OddLotShare
};

enum class Page {
    Main,
    Account,
    History,
    NewsHistory
};

class TradingSystem {
public:
    TradingSystem();

    ~TradingSystem();

    void Run();

private:

    static constexpr int screenWidth = 1600;
    static constexpr int screenHeight = 1000;
    static constexpr int minScreenWidth = 1200;
    static constexpr int minScreenHeight = 800;
    static constexpr double SHARES_PER_LOT = 1000.0;

    // =========================
    // Data
    // =========================

    std::vector<StockData> stocks;
    int selectedStock = 0;
    std::vector<MarketNews> activeNews;
    std::vector<std::pair<int, MarketNews>> newsHistory;
    double marketIndex = 1000.0;
    double marketOpenIndex = 1000.0;

    Account account;

    TradeMode tradeMode = TradeMode::WholeShare;

    Page page = Page::Main;

    // =========================
    // Order
    // =========================

    double orderQuantity = 1.0;

    std::string quantityInput = "1";

    bool editingQuantity = false;

    Rectangle quantityInputBox = {
        1290.0f,
        600.0f,
        160.0f,
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

    int windowedWidth = screenWidth;

    int windowedHeight = screenHeight;

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
        1600.0f,
        96.0f
    };

    Rectangle marketRow = {
        0.0f,
        96.0f,
        1600.0f,
        82.0f
    };

    Rectangle leftPanel = {
        40.0f,
        200.0f,
        280.0f,
        680.0f
    };

    Rectangle chartPanel = {
        340.0f,
        200.0f,
        850.0f,
        680.0f
    };

    Rectangle rightPanel = {
        1210.0f,
        200.0f,
        350.0f,
        680.0f
    };

    Rectangle bottomPanel = {
        40.0f,
        900.0f,
        1520.0f,
        70.0f
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
    Button newsButton;
    Button minusButton;

    Button plusButton;

    Button avatarButton;

    Button backButton;

    Button historyButton;

    // =========================
    // Setup
    // =========================

    void InitStocks();

    void SetupUI();

    void UpdateLayout();

    void ToggleFullscreenMode();

    void SeedInitialHistory(StockData& stock);

    StockData& CurrentStock();
    const StockData& CurrentStock() const;

    void GenerateNews();
    double GetPortfolioValue() const;

    // =========================
    // Update
    // =========================

    void Update(float dt);

    void UpdateMain(float dt);

    void UpdateAccount(float dt);

    void UpdateHistory(float dt);

    void UpdateNewsHistory(float dt);

    void ApplyQuantityInput();

    void UpdateTradeModeButtons();

    // =========================
    // Market Logic
    // =========================

    void AdvanceTime(float dt);

    void UpdatePrice(float dt);

    void PickTrendForNewDay();

    void NextDay();

    void UpdateMarketIndex();
    double GetMarketNewsIndexDrift() const;

    double GetTrendDriftPercent(TrendType trend);
    double GetRandomNoisePercent(const StockData& stock);
    double GetNewsAdjustmentPercent(const StockData& stock);

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

    void DrawNewsPanel(Rectangle area);

    void DrawIndustryPerformancePanel(Rectangle area);

    void DrawTradingPanel();

    void DrawNewsHistory();

    void DrawBottomSummary();

    void DrawPanel(Rectangle rect);

    void DrawInfoRow(
        const std::string& label,
        const std::string& value,
        float y,
        bool numberValue = false,
        Color color = Theme::TEXT,
        bool compact = false
    );

    void DrawLabelValue(
        const std::string& label,
        const std::string& value,
        Vector2 pos,
        bool numberValue = true,
        Color valueColor = Theme::TEXT
    );

    void DrawChart(Rectangle area);

    // =========================
    // Helpers
    // =========================

    double TotalAsset() const;

    double GetMarketIndex() const;
    double GetMarketIndexChangePercent() const;

    std::vector<std::pair<std::string, double>> GetTopGainers(int count = 3) const;
    std::vector<std::pair<std::string, double>> GetTopLosers(int count = 3) const;
    std::vector<std::pair<std::string, double>> GetIndustryPerformance() const;

    double RandomDouble(double low, double high);
    double RandomNormal();

    double GetNewsInfluenceForStock(const StockData& stock) const;
    double GetTrendBiasDirection(const StockData& stock) const;

    double GetLimitUpPrice(const StockData& stock) const;
    double GetLimitDownPrice(const StockData& stock) const;
    LimitStatus GetLimitStatus(const StockData& stock) const;
    std::string GetLimitStatusName(const StockData& stock) const;
    Color GetLimitStatusColor(const StockData& stock) const;

    double GetOrderShares() const;

    double GetEstimatedCost() const;

    std::string TimeString() const;

    int RandomInt(int low, int high);
};
