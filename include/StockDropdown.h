#pragma once

#include "raylib.h"
#include "StockData.h"
#include <vector>
#include <string>

class StockDropdown {
public:
    StockDropdown();
    explicit StockDropdown(Rectangle rect);

    void SetOptions(const std::vector<std::string>& items);
    void SetSelectedIndex(int index);
    void SetLimitStatus(LimitStatus status);
    int GetSelectedIndex() const;

    void Update();
    void Draw() const;

private:
    Rectangle bounds;
    std::vector<std::string> options;
    int selectedIndex = 0;
    LimitStatus limitStatus = LimitStatus::None;
    bool open = false;
};
