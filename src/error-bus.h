// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_ERROR_BUS_H
#define VIA_HAS_HEADER_ERROR_BUS_H

#include "common.h"

//  =============
// [ error-bus.h ]
//  =============
namespace via {

// Error level for error header text.
enum class CErrorLevel : uint8_t {
  INFO,
  WARNING,
  ERROR_,
};

// Error location info. Holds line, column and absolute offset information.
struct CErrorLocation {
  const size_t line = 0;
  const size_t column = 0;
  const size_t begin = 0;
  const size_t end = 0;

  constexpr CErrorLocation() = default;
  constexpr CErrorLocation(size_t l, size_t c, size_t b, size_t e)
    : line(l),
      column(c),
      begin(b),
      end(e) {}

  constexpr CErrorLocation(const Token& tok)
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
class CError final {
public:
  // Returns the error as a String.
  std::string to_string() const;

  CError(
    bool flt, const std::string& msg, TransUnitContext& ctx, CErrorLevel lvl, CErrorLocation pos
  )
    : is_flat(flt),
      message(msg),
      level(lvl),
      position(pos),
      ctx(ctx) {}

public:
  bool is_flat;
  std::string message;
  CErrorLevel level;
  CErrorLocation position;
  TransUnitContext& ctx;
};

/**
 * Error bus object. Buffers all error objects until CErrorBus::emit() is called or the object is
 * destructed.
 */
class CErrorBus final {
public:
  // Appends a user-specified level error to the buffer.
  void log(const CError& err);

  // Returns whether if the error bus contains fatal errors.
  bool has_error();

  // Clears all accumulated errors.
  void clear();

  // Emits all accumulated errors into stdout.
  void emit();

  // Appends a newline to the buffer.
  void new_line();

  // Custom destructor to ensure that the errors are emitted before destruction.
  ~CErrorBus();

private:
  struct Payload {
    CError error;
    std::string message;
  };

  std::vector<Payload> buffer;
};

} // namespace via

#endif
