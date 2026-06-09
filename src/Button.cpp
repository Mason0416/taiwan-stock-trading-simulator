#include "Button.h"
#include "GUI.h"

#include <algorithm>
#include <utility>

Button::Button()
    : bounds{0.0f, 0.0f, 0.0f, 0.0f},
      label(""),
      baseColor(Theme::GRAY_BUTTON),
      hoverColor(Theme::GRAY_HOVER),
      labelColor(Theme::TEXT),
      active(false),
      hoverT(0.0f) {}

Button::Button(
    Rectangle rect,
    std::string text,
    Color base,
    Color hover,
    Color textColor
)
    : bounds(rect),
      label(std::move(text)),
      baseColor(base),
      hoverColor(hover),
      labelColor(textColor),
      active(false),
      hoverT(0.0f) {}

void Button::SetActive(bool value) {
    active = value;
}

void Button::Update(float dt) {
    float target = IsHovered() ? 1.0f : 0.0f;
    hoverT += (target - hoverT) * std::min(1.0f, dt * 12.0f);
}

void Button::Draw() const {
    Color fill = active
        ? Theme::ACCENT_BLUE
        : LerpColor(baseColor, hoverColor, hoverT);

    DrawRectangleRounded(bounds, 0.12f, 10, fill);

    DrawRectangleRoundedLines(
        bounds,
        0.12f,
        10,
        active ? Theme::ACCENT_BLUE_HOVER : Theme::BORDER
    );

    float ui = GetUIScale();
    float size = std::min(22.0f, 19.0f * ui);
    size = std::min(size, bounds.height - 12.0f);
    float w = MeasureTextEx(gUIFont, label.c_str(), size, 0.0f).x;

    Vector2 pos = {
        bounds.x + (bounds.width - w) * 0.5f,
        bounds.y + (bounds.height - size) * 0.5f - 1.0f
    };

    DrawUI(label, pos, size, active ? WHITE : labelColor);
}

bool Button::IsHovered() const {
    return CheckCollisionPointRec(GetMousePosition(), bounds);
}

bool Button::IsClicked() const {
    return IsHovered() && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
}

Rectangle Button::GetBounds() const {
    return bounds;
}
