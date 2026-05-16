#include "GUI.h"

#include <algorithm>

#include <iomanip>

#include <sstream>

Font gFont;

bool gCustomFontLoaded = false;

Font LoadFontSafe(const char* path) {

    if (FileExists(path)) {

        Font loaded = LoadFont(path);

        if (loaded.texture.id != 0) {

            gCustomFontLoaded = true;

            return loaded;

        }

    }

    gCustomFontLoaded = false;

    return GetFontDefault();

}

void SetupFont() {

    gFont = LoadFontSafe(

        "resources/fonts/SFPRODISPLAYREGULAR.OTF"

    );

}

void UnloadFontSafe() {

    if (gCustomFontLoaded) {

        UnloadFont(gFont);

    }

}

void DrawUI(

    const std::string& text,

    Vector2 pos,

    float size,

    Color color

) {

    DrawTextEx(

        gFont,

        text.c_str(),

        pos,

        size,

        0.0f,

        color

    );

}

float TextWidth(

    const std::string& text,

    float size

) {

    return MeasureTextEx(

        gFont,

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