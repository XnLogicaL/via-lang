// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_CONTEXT_H
#define VIA_HAS_HEADER_CONTEXT_H

#include "common-defs.h"
#include "common-macros.h"

#include <arena.h>
#include <token.h>
#include <lexloc.h>
#include <sema_glob.h>
#include <sema_var.h>
#include <tvalue.h>

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
inline constexpr size_t STRING_ALLOCATOR_SIZE = 1024 * 1024 * 8;

inline constexpr size_t SEMA_ALLOCATOR_SIZE = 1024 * 1024 * 8;

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
struct Context {
public:
  // Returns platform info as a String in a static buffer.
  const char* get_platform_info();

  // Plain text file constructor.
  Context(const std::string& file_path, const std::string& file_source);
  // Binary file constructor.
  Context(const byte_stream_t& bytes);

public:
  // Relative path of the target file.
  const std::string file_path;
  // Plain text source of the target file.
  const std::string file_source;

  // Optimization level: 0-3
  size_t optimization_level = 0;
  size_t label_count = 0;

  std::vector<Token> tokens{};
  std::vector<AstNode*> ast{};
  std::vector<Instruction> bytecode{};
  std::vector<InstructionData> bytecode_data{};
  std::vector<Value> constants{};

  // Allocators
  ArenaAllocator astalloc{AST_ALLOCATOR_SIZE};
  ArenaAllocator stralloc{STRING_ALLOCATOR_SIZE};
  ArenaAllocator semalloc{SEMA_ALLOCATOR_SIZE};
};

} // namespace via

#endif
