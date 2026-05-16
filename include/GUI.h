#pragma once

#include "raylib.h"
#include "StockData.h"
#include <string>
#include <vector>

struct Theme {
    static constexpr Color BG = {7, 11, 18, 255};
    static constexpr Color TOP = {11, 17, 27, 255};
    static constexpr Color PANEL = {15, 23, 35, 255};
    static constexpr Color PANEL_2 = {20, 30, 45, 255};
    static constexpr Color PANEL_3 = {26, 38, 56, 255};
    static constexpr Color BORDER = {53, 75, 103, 255};
    static constexpr Color BORDER_SOFT = {35, 50, 70, 255};

    static constexpr Color TEXT = {235, 239, 247, 255};
    static constexpr Color MUTED = {143, 155, 173, 255};
    static constexpr Color DIM = {95, 108, 130, 255};

    static constexpr Color UP_RED = {210, 70, 85, 255};
    static constexpr Color DOWN_GREEN = {0, 180, 120, 255};

    static constexpr Color ACCENT_BLUE = {70, 100, 180, 255};
    static constexpr Color ACCENT_BLUE_HOVER = {90, 125, 210, 255};
    static constexpr Color GRAY_BUTTON = {52, 65, 83, 255};
    static constexpr Color GRAY_HOVER = {70, 84, 105, 255};
    static constexpr Color CHART_BG = {9, 15, 25, 255};
    static constexpr Color GRID = {48, 62, 80, 120};
};

class Button {
public:
    Button();

    Button(
        Rectangle rect,
        std::string text,
        Color base,
        Color hover,
        Color textColor
    );

    void SetActive(bool value);

    void Update(float dt);

    void Draw() const;

    bool IsHovered() const;

    bool IsClicked() const;

    Rectangle GetBounds() const;

private:
    Rectangle bounds {0.0f, 0.0f, 0.0f, 0.0f};

    std::string label;

    Color baseColor;
    Color hoverColor;
    Color labelColor;

    bool active = false;
    float hoverT = 0.0f;
};

class StockDropdown {
public:
    StockDropdown();

    explicit StockDropdown(Rectangle rect);

    void Update();

    void Draw() const;

private:
    Rectangle bounds {0.0f, 0.0f, 0.0f, 0.0f};

    std::vector<std::string> options;

    bool open = false;
};

void SetupFont();

void UnloadFontSafe();

void DrawUI(
    const std::string& text,
    Vector2 pos,
    float size,
    Color color
);

float TextWidth(
    const std::string& text,
    float size
);

Color LerpColor(
    Color a,
    Color b,
    float t
);

std::string FormatMoney(double value);

std::string FormatNumber(double value);

std::string FormatPercent(double value);

std::string FormatChangePercent(double value);

std::string FormatIntWithComma(int value);



std::string TrendName(TrendType trend);

Color TrendColor(TrendType trend);