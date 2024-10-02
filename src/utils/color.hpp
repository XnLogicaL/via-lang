#pragma once

#include <string>

namespace Dye {

#define RESET   "\033[0m"

    enum class TextColor {
        NONE = 0,
        BLACK = 30, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE,
        BRIGHT_BLACK = 90, BRIGHT_RED, BRIGHT_GREEN, BRIGHT_YELLOW,
        BRIGHT_BLUE, BRIGHT_MAGENTA, BRIGHT_CYAN, BRIGHT_WHITE
    };

    enum class BackgroundColor {
        NONE = 0,
        BLACK = 40, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE,
        BRIGHT_BLACK = 100, BRIGHT_RED, BRIGHT_GREEN, BRIGHT_YELLOW,
        BRIGHT_BLUE, BRIGHT_MAGENTA, BRIGHT_CYAN, BRIGHT_WHITE
    };

    enum class TextStyle {
        NONE = 0,
        BOLD = 1, UNDERLINE = 4, INVERT = 7
    };

    std::string style_text(
        TextColor text_color = TextColor::WHITE,
        BackgroundColor bg_color = BackgroundColor::NONE,
        TextStyle text_style = TextStyle::NONE,
        const std::string& message = "")
    {
        std::string ansi_code = "\033[";

        // Append text color if not NONE
        if (text_color != TextColor::NONE) {
            ansi_code += std::to_string(static_cast<int>(text_color)) + ";";
        }

        // Append background color if not NONE
        if (bg_color != BackgroundColor::NONE) {
            ansi_code += std::to_string(static_cast<int>(bg_color)) + ";";
        }

        // Append text style if not NONE
        if (text_style != TextStyle::NONE) {
            ansi_code += std::to_string(static_cast<int>(text_style)) + ";";
        }

        // Remove the last semicolon if any styles were added
        if (ansi_code.back() == ';') {
            ansi_code.pop_back();
        }

        ansi_code += "m"; // Complete the ANSI escape code

        return ansi_code + message + RESET; // Return styled message with reset
    }

} // namespace Dye