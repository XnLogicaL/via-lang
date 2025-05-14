// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_ERROR_BUS_H
#define VIA_HAS_HEADER_ERROR_BUS_H

#include "common.h"
#include "context.h"

#include <token.h>
#include <lexloc.h>
#include <color.h>

//  =============
// [ error_bus.h ]
//  =============
namespace via {

// Error level for error header text.
enum class CErrorLevel : uint8_t {
  INFO,
  WARNING,
  ERROR_,
};

/**
 * Error object. Includes error level, error location, error message, a reference to the appropriate
 * translation unit context, and a flag that decides whether if the error bus should print inline
 * information.
 */
struct CError {
  LexLocation loc;
  bool is_flat;
  std::string message;
  CErrorLevel level;
  Context& ctx;
};

/**
 * Error bus object. Buffers all error objects until CErrorBus::emit() is called or the object is
 * destructed.
 */
class CErrorBus final {
public:
  // Custom destructor to ensure that the errors are emitted before destruction.
  ~CErrorBus();

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

private:
  struct Payload {
    CError error;
    std::string message;
  };

  std::vector<Payload> buffer;
};

} // namespace via

#endif
