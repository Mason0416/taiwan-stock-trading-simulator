#pragma once

#include "raylib.h"
#include <string>

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
    Rectangle bounds;
    std::string label;

    Color baseColor;
    Color hoverColor;
    Color labelColor;

    bool active = false;
    float hoverT = 0.0f;
};