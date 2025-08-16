// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_CONTROL_PATH_H_
#define VIA_CORE_CONTROL_PATH_H_

#include <via/config.h>
#include <via/types.h>
#include "constexpr.h"
#include "parser/ast.h"

namespace via {

namespace sema {

enum class ControlPath {
  None,         // uninterrupted flow
  Return,       // returns from function
  Break,        // breaks a loop/switch
  Continue,     // continues a loop
  Unreachable,  // statement is never reached
};

ControlPath analyze_cpath(const ast::StmtNode* stmt);

}  // namespace sema

}  // namespace via

#endif
