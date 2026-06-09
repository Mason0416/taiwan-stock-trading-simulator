#include "GUI.h"

#include <algorithm>

#include <iomanip>

#include <sstream>

Font gUIFont;
Font gNumberFont;

bool gUIFontLoaded = false;
bool gNumberFontLoaded = false;

void SetupFont() {
    const char* path = "../resources/fonts/SFPRODISPLAYREGULAR.OTF";

    if (FileExists(path)) {
        TraceLog(LOG_INFO, "Font file found: %s", path);
    } else {
        TraceLog(LOG_WARNING, "Font file NOT found at expected path: %s", path);
    }

    // Try to load UI font
    if (FileExists(path)) {
        Font loaded = LoadFontEx(path, 48, nullptr, 0);
        if (loaded.texture.id != 0) {
            SetTextureFilter(loaded.texture, TEXTURE_FILTER_BILINEAR);
            gUIFont = loaded;
            gUIFontLoaded = true;
            TraceLog(LOG_INFO, "Loaded custom UI font '%s' (texture id=%d)", path, loaded.texture.id);
        } else {
            gUIFont = GetFontDefault();
            gUIFontLoaded = false;
            TraceLog(LOG_WARNING, "Failed to load UI font texture from '%s' - using default font", path);
        }
    } else {
        gUIFont = GetFontDefault();
        gUIFontLoaded = false;
        TraceLog(LOG_WARNING, "UI font file missing; using default font");
    }

    // Try to load numeric/font for numbers (same file but keep separate handle)
    if (FileExists(path)) {
        Font loadedNum = LoadFontEx(path, 48, nullptr, 0);
        if (loadedNum.texture.id != 0) {
            SetTextureFilter(loadedNum.texture, TEXTURE_FILTER_BILINEAR);
            gNumberFont = loadedNum;
            gNumberFontLoaded = true;
            TraceLog(LOG_INFO, "Loaded custom Number font '%s' (texture id=%d)", path, loadedNum.texture.id);
        } else {
            gNumberFont = GetFontDefault();
            gNumberFontLoaded = false;
            TraceLog(LOG_WARNING, "Failed to load Number font texture from '%s' - using default font", path);
        }
    } else {
        gNumberFont = GetFontDefault();
        gNumberFontLoaded = false;
        TraceLog(LOG_WARNING, "Number font file missing; using default font");
    }
}

void UnloadFontSafe() {
    if (gUIFontLoaded) {
        TraceLog(LOG_INFO, "Unloading custom UI font");
        UnloadFont(gUIFont);
        gUIFontLoaded = false;
    }
    if (gNumberFontLoaded) {
        TraceLog(LOG_INFO, "Unloading custom Number font");
        UnloadFont(gNumberFont);
        gNumberFontLoaded = false;
    }
}

float GetUIScale() {
    return std::clamp(static_cast<float>(GetScreenWidth()) / 1400.0f, 1.0f, 1.35f);
}

void DrawUI(

    const std::string& text,

    Vector2 pos,

    float size,

    Color color,

    bool numberFont

) {

    const Font& font = numberFont ? gNumberFont : gUIFont;
    DrawTextEx(
        font,
        text.c_str(),
        pos,
        size,
        0.0f,
        color
    );
}

void DrawNumber(
    const std::string& text,
    Vector2 pos,
    float size,
    Color color
) {
    DrawUI(text, pos, size, color, true);
}

float TextWidth(

    const std::string& text,

    float size,

    bool numberFont

) {

    const Font& font = numberFont ? gNumberFont : gUIFont;
    return MeasureTextEx(
        font,
        text.c_str(),
        size,
        0.0f
    ).x;

}

Color LerpColor(

    Color a,

    Color b,

    float t

) {

    t = std::clamp(t, 0.0f, 1.0f);

    return {

        static_cast<unsigned char>(a.r + (b.r - a.r) * t),

        static_cast<unsigned char>(a.g + (b.g - a.g) * t),

        static_cast<unsigned char>(a.b + (b.b - a.b) * t),

        static_cast<unsigned char>(a.a + (b.a - a.a) * t)

    };

}

std::string FormatMoney(double value) {

    std::ostringstream ss;

    ss << "$"

       << std::fixed

       << std::setprecision(2)

       << value;

    return ss.str();

}

std::string FormatNumber(double value) {

    std::ostringstream ss;

    ss << std::fixed

       << std::setprecision(2)

       << value;

    return ss.str();

}

std::string FormatPercent(double value) {

    std::ostringstream ss;

    ss << std::fixed

       << std::setprecision(2)

       << value

       << "%";

    return ss.str();

}

std::string FormatChangePercent(double value) {

    std::ostringstream ss;

    ss << (value >= 0.0 ? "+" : "")

       << std::fixed

       << std::setprecision(2)

       << value

       << "%";

    return ss.str();

}

std::string FormatIntWithComma(int value) {

    std::string s = std::to_string(value);

    std::string out;

    int count = 0;

    for (int i = s.size() - 1; i >= 0; --i) {

        out.push_back(s[i]);

        count++;

        if (count == 3 && i != 0) {

            out.push_back(',');

            count = 0;

        }

    }

    std::reverse(out.begin(), out.end());

    return out;

}

std::string TrendName(TrendType trend) {
    switch (trend) {
        case TrendType::Bullish: return "Bullish";
        case TrendType::Bearish: return "Bearish";
        default: return "Neutral";
    }
}

Color TrendColor(TrendType trend) {
    switch (trend) {
        case TrendType::Bullish: return Theme::UP_RED;
        case TrendType::Bearish: return Theme::DOWN_GREEN;
        default: return Theme::MUTED;
    }
}
