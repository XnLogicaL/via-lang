// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "visitor.h"

namespace via {

using enum comp_err_lvl;

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

value_obj construct_constant(lit_expr_node& literal_node) {
  using enum value_type;
  return std::visit(
    [](auto&& val) -> value_obj {
      using T = std::decay_t<decltype(val)>;

      if constexpr (std::is_same_v<T, int>) {
        return value_obj(val);
      }
      else if constexpr (std::is_same_v<T, bool>) {
        return value_obj(val);
      }
      else if constexpr (std::is_same_v<T, float>) {
        return value_obj(val);
      }
      else if constexpr (std::is_same_v<T, std::string>) {
        string_obj* tstring = new string_obj(nullptr, val.data());
        return value_obj(string, static_cast<void*>(tstring));
      }
      else if constexpr (std::is_same_v<T, std::monostate>) {
        return value_obj();
      }

      vl_unreachable;
    },
    literal_node.value
  );
}

void node_visitor_base::compiler_error(size_t begin, size_t end, const std::string& message) {
  auto lc_info = get_line_and_column(unit_ctx.file_source, begin);

  visitor_failed = true;
  err_bus.log({false, message, unit_ctx, ERROR_, {lc_info.first, lc_info.second, begin, end}});
}

void node_visitor_base::compiler_error(const token& token, const std::string& message) {
  visitor_failed = true;
  err_bus.log({false, message, unit_ctx, ERROR_, token});
}

void node_visitor_base::compiler_error(const std::string& message) {
  visitor_failed = true;
  err_bus.log({true, message, unit_ctx, ERROR_, {}});
}

void node_visitor_base::compiler_warning(size_t begin, size_t end, const std::string& message) {
  auto lc_info = get_line_and_column(unit_ctx.file_source, begin);
  err_bus.log({false, message, unit_ctx, WARNING, {lc_info.first, lc_info.second, begin, end}});
}

void node_visitor_base::compiler_warning(const token& token, const std::string& message) {
  err_bus.log({false, message, unit_ctx, WARNING, token});
}

void node_visitor_base::compiler_warning(const std::string& message) {
  err_bus.log({true, message, unit_ctx, WARNING, {}});
}

void node_visitor_base::compiler_info(size_t begin, size_t end, const std::string& message) {
  auto lc_info = get_line_and_column(unit_ctx.file_source, begin);
  err_bus.log({false, message, unit_ctx, INFO, {lc_info.first, lc_info.second, begin, end}});
}

void node_visitor_base::compiler_info(const token& token, const std::string& message) {
  err_bus.log({false, message, unit_ctx, INFO, token});
}

void node_visitor_base::compiler_info(const std::string& message) {
  err_bus.log({true, message, unit_ctx, INFO, {}});
}

} // namespace via
