// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_CONTEXT_H
#define VIA_HAS_HEADER_CONTEXT_H

#include "common-defs.h"
#include "common-macros.h"

#include <arena.h>
#include <lex/token.h>
#include <parse/ast-base.h>
#include <codegen/globals.h>
#include <codegen/stack.h>
#include <interpreter/tvalue.h>

/**
 * @namespace via
 * @brief Main namespace of the project. Contains all but macros
 * @defgroup via_namespace
 */
namespace via {

/**
 * @brief Abstract Syntax Tree allocator size in bytes
 * @ingroup via_namespace
 */
inline constexpr size_t AST_ALLOCATOR_SIZE = 1024 * 1024 * 8;

/**
 * @brief String allocator size in bytes
 * @ingroup via_namespace
 */
inline constexpr size_t STRING_ALLOCATOR_SIZE = 1024 * 64;

/**
 * @brief Dynamic container that holds a sequence of bytes.
 * @ingroup via_namespace
 * @details Used for constructing translation unit context objects from binary files.
 */
using byte_stream_t = std::vector<uint8_t>;

// Context objects forward declarations.
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
  size_t label_count = 0;

  std::vector<Token> tokens{};
  std::vector<StmtNodeBase*> ast{};
  std::vector<Instruction> bytecode{};
  std::vector<InstructionData> bytecode_data{};
  std::vector<Value> constants{};
  std::vector<std::vector<StmtNodeBase*>> defered_stmts;

  // Allocators
  ArenaAllocator ast_allocator{AST_ALLOCATOR_SIZE};
  ArenaAllocator string_allocator{STRING_ALLOCATOR_SIZE};

  GlobalHolder globals;
  CompilerFunctionStack function_stack;
};

} // namespace via

#endif
