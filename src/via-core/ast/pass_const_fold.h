// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_AST_PASS_CONST_FOLD_H_
#define VIA_CORE_AST_PASS_CONST_FOLD_H_

#include <via/config.h>
#include <via/types.h>
#include "pass.h"
#include "sema/const_value.h"
#include "sema/constexpr.h"
#include "visitor.h"

namespace via {

namespace ast {

class PassConstFold final : public Pass {
 public:
  StmtNode* apply(StmtNode* stmt) override;
};

}  // namespace ast

}  // namespace via

#endif
