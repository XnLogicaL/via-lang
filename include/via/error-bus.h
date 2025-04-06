// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef VIA_HAS_HEADER_ERROR_BUS_H
#define VIA_HAS_HEADER_ERROR_BUS_H

#include "common.h"

namespace via {

// Error level for error header text.
enum class comp_err_lvl : uint8_t {
  INFO,
  WARNING,
  ERROR_,
};

// Error location info. Holds line, column and absolute offset information.
struct comp_err_pos {
  const size_t line = 0;
  const size_t column = 0;
  const size_t begin = 0;
  const size_t end = 0;

  comp_err_pos() = default;
  comp_err_pos(size_t l, size_t c, size_t b, size_t e)
    : line(l),
      column(c),
      begin(b),
      end(e) {}

  comp_err_pos(const token& tok)
    : line(tok.line),
      column(tok.offset),
      begin(tok.position),
      end(tok.position + tok.lexeme.length()) {}
};

/**
 * Error object. Includes error level, error location, error message, a reference to the appropriate
 * translation unit context, and a flag that decides whether if the error bus should print inline
 * information.
 */
class compile_error final {
public:
  // Returns the error as a string.
  std::string to_string() const;

  compile_error(
    bool flt, const std::string& msg, trans_unit_context& ctx, comp_err_lvl lvl, comp_err_pos pos
  )
    : is_flat(flt),
      message(msg),
      ctx(ctx),
      level(lvl),
      position(pos) {}

public:
  bool is_flat;
  std::string message;
  trans_unit_context& ctx;
  comp_err_lvl level;
  comp_err_pos position;
};

/**
 * Error bus object. Buffers all error objects until error_bus::emit() is called or the object is
 * destructed.
 */
class error_bus final {
public:
  // Appends a user-specified level error to the bus.
  void log(const compile_error& err);

  // Returns whether if the error bus contains fatal errors.
  bool has_error();

  // Clears all accumulated errors.
  void clear();

  // Emits all accumulated errors into stdout.
  void emit();

  // Custom destructor to ensure that the errors are emitted before destruction.
  ~error_bus();

private:
  std::vector<compile_error> buffer;
};

} // namespace via

#endif
