#include "GUI.h"

StockDropdown::StockDropdown() = default;

StockDropdown::StockDropdown(Rectangle rect)
    : bounds(rect) {

    options.push_back("Aurora Tech (AUR)");
}

void StockDropdown::Update() {

    if (!IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        return;
    }

    if (CheckCollisionPointRec(GetMousePosition(), bounds)) {
        open = !open;
    }
    else {
        open = false;
    }
}

void StockDropdown::Draw() const {

    DrawRectangleRounded(
        bounds,
        0.10f,
        10,
        Theme::PANEL_3
    );

    DrawRectangleRoundedLines(
        bounds,
        0.10f,
        10,
        Theme::BORDER
    );

    DrawUI(
        options[0],
        {bounds.x + 14.0f, bounds.y + 12.0f},
        20.0f,
        Theme::TEXT
    );

    DrawUI(
        open ? "v" : ">",
        {bounds.x + bounds.width - 25.0f, bounds.y + 12.0f},
        16.0f,
        Theme::MUTED
    );

    if (open) {

        Rectangle list = {
            bounds.x,
            bounds.y + bounds.height + 4.0f,
            bounds.width,
            bounds.height
        };

        DrawRectangleRounded(
            list,
            0.10f,
            10,
            Theme::PANEL_2
        );

        DrawRectangleRoundedLines(
            list,
            0.10f,
            10,
            Theme::BORDER
        );

        DrawUI(
            options[0],
            {list.x + 14.0f, list.y + 12.0f},
            20.0f,
            Theme::TEXT
        );
    }
}