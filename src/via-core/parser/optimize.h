// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_PARSER_OPTIMIZE_H_
#define VIA_PARSER_OPTIMIZE_H_

#include <via/config.h>
#include <via/types.h>
#include "ast.h"
#include "parser.h"

namespace via {

class OptimizationPass {
 public:
  OptimizationPass(HeapAllocator& alloc) : alloc(alloc) {}
  virtual void apply(AstBuf& ast) = 0;

 protected:
  HeapAllocator& alloc;
};

}  // namespace via

#endif
