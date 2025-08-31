// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_IR_BUILDER_H_
#define VIA_CORE_IR_BUILDER_H_

#include <via/config.h>
#include <via/types.h>
#include "ir.h"

namespace via
{

class Module;

namespace ir
{

class Builder final
{
 public:
  Builder(via::Module* m, const SyntaxTree& ast, DiagContext& diags)
      : mModule(m), mAst(ast), mDiags(diags)
  {}

 public:
  IrTree build();

 private:
  via::Module* mModule;
  const SyntaxTree& mAst;
  DiagContext& mDiags;
};

}  // namespace ir

}  // namespace via

#endif
