// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_SEMACPATH_H
#define VIA_SEMACPATH_H

#include "common.h"
#include "ast.h"

namespace via {

enum ControlPathResult {
  CPR_NONE,        // uninterrupted flow
  CPR_RETURN,      // returns from function
  CPR_BREAK,       // breaks a loop/switch
  CPR_CONTINUE,    // continues a loop
  CPR_UNREACHABLE, // statement is never reached
};

ControlPathResult sema_cpath_analyze(const StmtNode* stmt);

} // namespace via

#endif
