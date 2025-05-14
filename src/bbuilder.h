// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file bytecode-builder.h
 * @brief Declares the BytecodeBuilder class along with utility
 */
#ifndef VIA_HAS_HEADER_COMPILER_H
#define VIA_HAS_HEADER_COMPILER_H

#include "common.h"
#include <ast.h>
#include <visitor.h>
#include <types.h>
#include <instruction.h>
#include <state.h>

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

/** @} */

/**
 * @class BytecodeBuilder
 * @brief Builds bytecode from the abstract syntax tree found inside the translation unit context.
 */
class BytecodeBuilder final {
public:
  BytecodeBuilder(Context& lctx)
    : ctx(lctx) {}

  /**
   * @brief Bytecode building entry point
   * @return Fail status
   */
  bool generate();

private:
  /**
   * @brief Prepares the builder and contexts for code generation
   */
  void codegen_prep();

  /**
   * @brief Emits a "successful exit" instruction to guarantee that the program exits safely
   */
  void insert_exit0_instruction();

private:
  VisitorContext ctx;
};

} // namespace via

/** @} */

#endif
