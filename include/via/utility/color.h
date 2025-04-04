// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_color_h
#define vl_has_header_color_h

#include "common.h"

namespace via::utils {

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

enum class style {
  reset = 0,
  bold = 1,
  faint = 2,
  italic = 3,
  underline = 4,
};

// Function to wrap a string with ANSI escape codes
vl_implement std::string apply_color(
  const std::string& text, fg_color fg, bg_color bg = bg_color::black, style style = style::reset
) {
  return "\033[" + std::to_string(static_cast<int>(style)) + ";" +
         std::to_string(static_cast<int>(fg)) + ";" + std::to_string(static_cast<int>(bg)) + "m" +
         text + "\033[0m";
}

} // namespace via::utils

#endif
