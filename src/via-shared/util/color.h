// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_COLOR_H_
#define VIA_COLOR_H_

#include "via/config.h"

namespace via {

// ANSI text foreground color codes.
enum class FGColor {
  Red = 31,
  Green = 32,
  Blue = 34,
  Yellow = 33,
  Magenta = 35,
  Cyan = 36,
  White = 37,
  Black = 30
};

// ANSI text background color codes.
enum class BGColor {
  Red = 41,
  Green = 42,
  Blue = 44,
  Yellow = 43,
  Magenta = 45,
  Cyan = 46,
  White = 47,
  Black = 40
};

// ANSI text style color codes.
enum class Style {
  Reset = 0,
  Bold = 1,
  Faint = 2,
  Italic = 3,
  Underline = 4,
};

// Function to wrap a String with ANSI escape codes
inline String apply_color(
  String string,
  FGColor foreground_color,
  BGColor background_color = BGColor::Black,
  Style style = Style::Reset
) {
  // Construct ANSI escape code and apply the color formatting to the text
  return "\033[" + std::to_string(static_cast<int>(style)) + ";"
    + std::to_string(static_cast<int>(foreground_color)) + ";"
    + std::to_string(static_cast<int>(background_color)) + "m" + string + "\033[0m";
}

} // namespace via

#endif
