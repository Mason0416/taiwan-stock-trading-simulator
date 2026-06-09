#include "GUI.h"
#include "StockDropdown.h"

#include <algorithm>
#include <cmath>

StockDropdown::StockDropdown() = default;

StockDropdown::StockDropdown(Rectangle rect)
    : bounds(rect) {
}

void StockDropdown::SetOptions(const std::vector<std::string>& items) {
    options = items;
    if (selectedIndex >= static_cast<int>(options.size())) {
        selectedIndex = 0;
    }
}

void StockDropdown::SetSelectedIndex(int index) {
    if (index >= 0 && index < static_cast<int>(options.size())) {
        selectedIndex = index;
    }
}

void StockDropdown::SetLimitStatus(LimitStatus status) {
    limitStatus = status;
}

int StockDropdown::GetSelectedIndex() const {
    return selectedIndex;
}

void StockDropdown::Update() {
    if (!IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        return;
    }

    Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, bounds)) {
        open = !open;
        return;
    }

    if (!open) {
        return;
    }

    Rectangle list = {
        bounds.x,
        bounds.y + bounds.height + 4.0f,
        bounds.width,
        bounds.height * static_cast<float>(options.size())
    };

    if (CheckCollisionPointRec(mouse, list)) {
        int index = static_cast<int>((mouse.y - list.y) / bounds.height);
        if (index >= 0 && index < static_cast<int>(options.size())) {
            selectedIndex = index;
        }
    }

    open = false;
}

void StockDropdown::Draw() const {
    const float ui = GetUIScale();
    const float textSize = std::min(22.0f * ui, bounds.height - 12.0f);
    const float arrowSize = std::min(18.0f * ui, bounds.height - 14.0f);
    const bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);

    Color idleFill = Theme::PANEL_3;
    Color hoverFill = Theme::PANEL_2;
    Color border = Theme::BORDER;

    if (limitStatus == LimitStatus::LimitUp) {
        idleFill = Theme::LIMIT_UP_DARK;
        hoverFill = Theme::LIMIT_UP_HOVER;
        border = Theme::UP_RED;
    } else if (limitStatus == LimitStatus::LimitDown) {
        idleFill = Theme::LIMIT_DOWN_DARK;
        hoverFill = Theme::LIMIT_DOWN_HOVER;
        border = Theme::DOWN_GREEN;
    }

    Color fill = hovered || open ? hoverFill : idleFill;
    if (limitStatus != LimitStatus::None) {
        float pulse = 0.5f + 0.5f * std::sin(static_cast<float>(GetTime()) * 4.0f);
        fill = LerpColor(fill, hoverFill, 0.10f + pulse * 0.10f);
    }

    DrawRectangleRounded(bounds, 0.10f, 10, fill);
    DrawRectangleRoundedLines(bounds, 0.10f, 10, border);

    if (!options.empty()) {
        DrawUI(options[selectedIndex], {bounds.x + 14.0f, bounds.y + 11.0f}, textSize, WHITE);
    }

    DrawUI(open ? "v" : ">", {bounds.x + bounds.width - 25.0f, bounds.y + 12.0f}, arrowSize, WHITE);

    if (!open || options.empty()) {
        return;
    }

    Rectangle list = {
        bounds.x,
        bounds.y + bounds.height + 4.0f,
        bounds.width,
        bounds.height * static_cast<float>(options.size())
    };

    DrawRectangleRounded(list, 0.10f, 10, Theme::PANEL_2);
    DrawRectangleRoundedLines(list, 0.10f, 10, Theme::BORDER);

    for (int i = 0; i < static_cast<int>(options.size()); ++i) {
        Rectangle optionRect = {
            list.x,
            list.y + bounds.height * i,
            list.width,
            bounds.height
        };

        if (i == selectedIndex) {
            DrawRectangleRec(optionRect, Theme::PANEL_3);
        }

        DrawUI(options[i], {optionRect.x + 14.0f, optionRect.y + 11.0f}, textSize,
               i == selectedIndex ? Theme::ACCENT_BLUE : Theme::TEXT);
    }
}
