// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_ERROR_BUS_H
#define _VIA_ERROR_BUS_H

#include "common.h"

VIA_NAMESPACE_BEGIN

enum class CompilerErrorLevel : uint8_t {
  INFO,
  WARNING,
  ERROR_,
};

struct CompilerErrorPosition {
  const size_t line = 0;
  const size_t column = 0;
  const size_t begin = 0;
  const size_t end = 0;

  VIA_DEFAULT_CONSTRUCTOR(CompilerErrorPosition);

  CompilerErrorPosition(size_t l, size_t c, size_t b, size_t e)
    : line(l),
      column(c),
      begin(b),
      end(e) {}

  CompilerErrorPosition(const Token& tok)
    : line(tok.line),
      column(tok.offset),
      begin(tok.position),
      end(tok.position + tok.lexeme.length()) {}
};

class CompilerError final {
public:
  // Returns the error as a string.
  std::string to_string() const;

  CompilerError(
    bool flt,
    const std::string& msg,
    TransUnitContext& ctx,
    CompilerErrorLevel lvl,
    CompilerErrorPosition pos
  )
    : is_flat(flt),
      message(msg),
      ctx(ctx),
      level(lvl),
      position(pos) {}

public:
  bool is_flat;

  std::string message;

  TransUnitContext& ctx;

  CompilerErrorLevel level;
  CompilerErrorPosition position;
};

class ErrorBus final {
public:
  // Appends a user-specified level error to the bus.
  void log(const CompilerError& err);

  // Returns whether if the error bus contains fatal errors.
  bool has_error();

  // Clears all accumulated errors.
  void clear();

  // Emits all accumulated errors into stdout.
  void emit();

  VIA_CUSTOM_DESTRUCTOR(ErrorBus);

private:
  std::vector<CompilerError> buffer;
};

VIA_NAMESPACE_END

#endif
