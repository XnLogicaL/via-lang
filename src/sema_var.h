// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file stack.h
 * @brief Declares stack structures used by the compiler
 */
#ifndef VIA_HAS_HEADER_STACK_H
#define VIA_HAS_HEADER_STACK_H

#include "common.h"

#include <ast.h>
#include <instruction.h>

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

struct Context;

namespace sema {

inline constexpr int F_SEMA_CONST = 32 >> 1;

struct SemaVar {
  const int flags;
  const char* id;
  const AstNode* const decl;
  const AstNode* const type;
  AstNode* val;
};

struct SemaFunc {
  const char* id;
  const AstNode* const decl;
};

struct SemaStk {
  size_t sp;
  SemaFunc* frames;
};

} // namespace sema

} // namespace via

/** @} */

#endif
