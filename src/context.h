// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_CONTEXT_H
#define VIA_HAS_HEADER_CONTEXT_H

#include "common-defs.h"
#include "common-macros.h"

#include <lex/token.h>
#include <compiler/globals.h>
#include <compiler/bytecode.h>
#include <compiler/constant.h>
#include <compiler/stack.h>

namespace via {

/**
 * Dynamic container that holds a sequence of bytes.
 * Used for constructing translation unit context objects from binary files.
 */
using byte_stream_t = std::vector<uint8_t>;

// Context objects forward declarations.
class TokenHolder;
class SyntaxTree;
class BytecodeHolder;
class ConstantHolder;
class CompilerVariableStack;
class CompilerFunctionStack;
class GlobalHolder;

// Per translation unit context.
class TransUnitContext final {
public:
  // Resets the translation unit context.
  void clear();

  // Encodes the translation unit onto a binary byte stream.
  byte_stream_t encode();

  // Returns platform info as a String in a static buffer.
  const char* get_platform_info();

  // Plain text file constructor.
  TransUnitContext(const std::string& file_path, const std::string& file_source);
  // Binary file constructor.
  TransUnitContext(const byte_stream_t& bytes);

public:
  // Relative path of the target file.
  const std::string file_path;
  // Plain text source of the target file.
  const std::string file_source;

  // Optimization level: 0-3
  size_t optimization_level = 0;

  std::unique_ptr<TokenHolder> tokens;
  std::unique_ptr<SyntaxTree> ast;
  std::unique_ptr<BytecodeHolder> bytecode;
  std::unique_ptr<ConstantHolder> constants;

  struct {
    size_t label_count;

    std::unique_ptr<CompilerFunctionStack> function_stack;
    std::stack<std::vector<StmtNodeBase*>> defered_stmts;
    std::unique_ptr<GlobalHolder> globals;
  } internal;
};

} // namespace via

#endif
