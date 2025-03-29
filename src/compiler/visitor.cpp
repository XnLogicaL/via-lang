// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "visitor.h"

VIA_NAMESPACE_BEGIN

using enum CompilerErrorLevel;

// Helper function to return line and column number from a character offset
std::pair<size_t, size_t> get_line_and_column(const std::string& source, size_t offset) {
  size_t line = 1;
  size_t column = 1;

  // Iterate over the string until we reach the offset
  for (size_t i = 0; i < offset; ++i) {
    if (source[i] == '\n') {
      ++line;
      column = 1; // Reset column to 1 for a new line
    }
    else {
      ++column;
    }
  }

  return {line, column};
}

TValue construct_constant(LiteralExprNode& literal_node) {
  using enum ValueType;
  return std::visit(
    [](auto&& val) -> TValue {
      using T = std::decay_t<decltype(val)>;

      if constexpr (std::is_same_v<T, int>) {
        return TValue(val);
      }
      else if constexpr (std::is_same_v<T, bool>) {
        return TValue(val);
      }
      else if constexpr (std::is_same_v<T, float>) {
        return TValue(val);
      }
      else if constexpr (std::is_same_v<T, std::string>) {
        TString* tstring = new TString(nullptr, val.data());
        return TValue(string, static_cast<void*>(tstring));
      }

      VIA_UNREACHABLE;
    },
    literal_node.value
  );
}

void NodeVisitor::compiler_error(size_t begin, size_t end, const std::string& message) {
  auto lc_info = get_line_and_column(unit_ctx.file_source, begin);

  visitor_failed = true;
  err_bus.log({false, message, unit_ctx, ERROR_, {lc_info.first, lc_info.second, begin, end}});
}

void NodeVisitor::compiler_error(const Token& token, const std::string& message) {
  visitor_failed = true;
  err_bus.log({false, message, unit_ctx, ERROR_, token});
}

void NodeVisitor::compiler_error(const std::string& message) {
  visitor_failed = true;
  err_bus.log({true, message, unit_ctx, ERROR_, {}});
}

void NodeVisitor::compiler_warning(size_t begin, size_t end, const std::string& message) {
  auto lc_info = get_line_and_column(unit_ctx.file_source, begin);
  err_bus.log({false, message, unit_ctx, WARNING, {lc_info.first, lc_info.second, begin, end}});
}

void NodeVisitor::compiler_warning(const Token& token, const std::string& message) {
  err_bus.log({false, message, unit_ctx, WARNING, token});
}

void NodeVisitor::compiler_warning(const std::string& message) {
  err_bus.log({true, message, unit_ctx, WARNING, {}});
}

void NodeVisitor::compiler_info(size_t begin, size_t end, const std::string& message) {
  auto lc_info = get_line_and_column(unit_ctx.file_source, begin);
  err_bus.log({false, message, unit_ctx, INFO, {lc_info.first, lc_info.second, begin, end}});
}

void NodeVisitor::compiler_info(const Token& token, const std::string& message) {
  err_bus.log({false, message, unit_ctx, INFO, token});
}

void NodeVisitor::compiler_info(const std::string& message) {
  err_bus.log({true, message, unit_ctx, INFO, {}});
}

VIA_NAMESPACE_END
