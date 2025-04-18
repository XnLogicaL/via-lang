// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_COLOR_H
#define VIA_HAS_HEADER_COLOR_H

#include "common.h"

namespace via::utils {

// ANSI text foreground color codes.
enum class fg_color {
  red = 31,
  green = 32,
  blue = 34,
  yellow = 33,
  magenta = 35,
  cyan = 36,
  white = 37,
  black = 30
};

// ANSI text background color codes.
enum class bg_color {
  red = 41,
  green = 42,
  blue = 44,
  yellow = 43,
  magenta = 45,
  cyan = 46,
  white = 47,
  black = 40
};

// ANSI text style color codes.
enum class style {
  reset = 0,
  bold = 1,
  faint = 2,
  italic = 3,
  underline = 4,
};

// Function to wrap a String with ANSI escape codes
inline std::string apply_color(
  const std::string& text, fg_color fg, bg_color bg = bg_color::black, style s = style::reset
) {
  // Construct ANSI escape code and apply the color formatting to the text
  return "\033[" + std::to_string(static_cast<int>(s)) + ";" + std::to_string(static_cast<int>(fg))
    + ";" + std::to_string(static_cast<int>(bg)) + "m" + text + "\033[0m";
}

} // namespace via::utils

#endif
