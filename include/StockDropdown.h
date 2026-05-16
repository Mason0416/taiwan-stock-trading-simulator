#pragma once

#include "raylib.h"
#include <vector>
#include <string>

class StockDropdown {
public:
    StockDropdown();
    explicit StockDropdown(Rectangle rect);

    void Update();
    void Draw() const;

private:
    Rectangle bounds;

    std::vector<std::string> options;

    bool open = false;
};