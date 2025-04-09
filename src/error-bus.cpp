// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "error-bus.h"
#include "context.h"
#include "ast.h"
#include "constant.h"
#include "bytecode.h"
#include "token.h"
#include "globals.h"
#include "stack.h"
#include "color.h"

namespace via {

using namespace utils;
using enum CErrorLevel;

struct LocalOffset {
  size_t line;
  size_t offset;
};

// ==========================================================================================
// Utility
//
// Returns each line in the source as a vector of strings.
std::vector<std::string> get_lines(const std::string& source) {
  std::istringstream stream(source);
  std::vector<std::string> lines;
  std::string line;

  while (std::getline(stream, line)) {
    lines.push_back(line);
  }

  return lines;
}

// Returns a slice (range) of lines from the source given zero-indexed begin and end.
// 'end' is exclusive.
std::vector<std::string> get_lines_in_range(const std::string& source, size_t begin, size_t end) {
  auto lines = get_lines(source);
  if (begin > lines.size())
    begin = lines.size();
  if (end > lines.size())
    end = lines.size();
  return std::vector<std::string>(lines.begin() + begin, lines.begin() + end);
}

// Converts an absolute character offset to a (line, column) pair.
LocalOffset localize_offset(const std::string& source, size_t abs_offset) {
  size_t line_number = 1;
  size_t line_start = 0;

  for (size_t i = 0; i < source.size() && i < abs_offset; ++i) {
    if (source[i] == '\n') {
      ++line_number;
      line_start = i + 1;
    }
  }

  size_t localized_offset = abs_offset >= line_start ? abs_offset - line_start : 0;
  return {line_number, localized_offset};
}

// Returns a header string (with ANSI colors) for a given error level.
std::string get_error_level_header(const CErrorLevel& lvl) {
  switch (lvl) {
  case INFO:
    return apply_color("info: ", fg_color::cyan, bg_color::black, style::bold);
  case WARNING:
    return apply_color("warning: ", fg_color::yellow, bg_color::black, style::bold);
  case ERROR_:
    return apply_color("error: ", fg_color::red, bg_color::black, style::bold);
  default:
    VIA_UNREACHABLE();
  }
}

// ==========================================================================================
// CError
std::string CError::to_string() const {
  std::string header = get_error_level_header(level);

  if (is_flat) {
    return header + message + '\n';
  }

  // Print header with file, line, and column information.
  std::ostringstream oss;
  oss << header << message << '\n';

  // Localize absolute offsets into (line, offset) values.
  LocalOffset local_begin = localize_offset(ctx.file_source, position.begin);
  LocalOffset local_end = localize_offset(ctx.file_source, position.end);

  // Adjust line indices: our get_lines_in_range expects 0-indexed line numbers.
  // Note: local_begin.line and local_end.line are 1-indexed.
  auto lines = get_lines_in_range(ctx.file_source, local_begin.line - 1, local_end.line);

  size_t pos = 0;
  for (const std::string& line : lines) {
    // Calculate the actual line number in the source.
    size_t current_line_number = local_begin.line + pos;
    std::string line_number_str = std::to_string(current_line_number);

    std::string underline;
    if (lines.size() == 1) {
      // Single-line error: underline from local_begin.offset to local_end.offset.
      underline = std::string(local_begin.offset, ' ')
        + std::string(local_end.offset - local_begin.offset, '^')
        + std::string(line.size() > local_end.offset ? line.size() - local_end.offset : 0, ' ');
    }
    else {
      // Multi-line error.
      if (pos == 0) {
        // First line: underline from local_begin.offset to end of line.
        underline =
          std::string(local_begin.offset, ' ') + std::string(line.size() - local_begin.offset, '^');
      }
      else if (pos == lines.size() - 1) {
        // Last line: underline from beginning of line up to local_end.offset.
        underline = std::string(local_end.offset, '^')
          + std::string(line.size() > local_end.offset ? line.size() - local_end.offset : 0, ' ');
      }
      else {
        // Intermediate lines: fully underline the entire line.
        underline = std::string(line.size(), '^');
      }
    }

    underline = apply_color(underline, fg_color::red, bg_color::black, style::bold);

    oss << line_number_str << " | " << line << '\n';
    oss << std::string(line_number_str.size(), ' ') << " | " << underline << '\n';

    ++pos;
  }

  return oss.str();
}

// ==========================================================================================
// CErrorBus
void CErrorBus::log(const CError& err) {
  buffer.push_back({err, err.to_string()});
}

bool CErrorBus::has_error() {
  for (const Payload& payload : buffer) {
    if (payload.error.level == ERROR_) {
      return true;
    }
  }

  return false;
}

void CErrorBus::clear() {
  buffer.clear();
}

void CErrorBus::emit() {
  std::string current_file;

  for (const Payload& payload : buffer) {
    if (!payload.error.is_flat && current_file != payload.error.ctx.file_path) {
      current_file = payload.error.ctx.file_path;
      std::cout << "in file " << current_file << ":\n";
    }

    std::cout << payload.message;
  }

  std::cout << std::flush;

  clear();
}

void CErrorBus::new_line() {
  static TransUnitContext dummy_ctx({});
  buffer.push_back({
    CError(true, "", dummy_ctx, INFO, {0, 0, 0, 0}),
    "──────────────────────────────────────────────\n",
  });
}

CErrorBus::~CErrorBus() {
  emit();
}

} // namespace via
