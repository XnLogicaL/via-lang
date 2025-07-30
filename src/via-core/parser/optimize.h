// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_PARSER_OPTIMIZE_H_
#define VIA_PARSER_OPTIMIZE_H_

#include <via/config.h>
#include "ast.h"
#include "parser.h"

namespace via {

namespace core {

namespace parser {

class OptimizationPass {
 public:
  OptimizationPass(HeapAllocator& alloc) : alloc(alloc) {}
  virtual void apply(AstBuf& ast) const = 0;

 protected:
  HeapAllocator& alloc;
};

}  // namespace parser

}  // namespace core

}  // namespace via

#endif
