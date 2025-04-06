// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef VIA_HAS_HEADER_CONTEXT_H
#define VIA_HAS_HEADER_CONTEXT_H

#include "common-defs.h"
#include "common-macros.h"
#include "compiler/globals.h"
#include "lex/token.h"

#define VIA_CTX_VERBOSE int(1 << 0)

namespace via {

/**
 * Dynamic container that holds a sequence of bytes.
 * Used for constructing translation unit context objects from binary files.
 */
using byte_stream_t = std::vector<uint8_t>;

// Context objects forward declarations.
class token_stream;
class syntax_tree;
class bytecode_holder;
class constant_holder;
class compiler_variable_stack;
class compiler_function_stack;
class global_holder;

// Per translation unit context.
class trans_unit_context final {
public:
  // Resets the translation unit context.
  void clear();

  // Encodes the translation unit onto a binary byte stream.
  byte_stream_t encode();

  // Returns platform info as a string in a static buffer.
  const char* get_platform_info();

  // Plain text file constructor.
  trans_unit_context(const std::string& file_path, const std::string& file_source);
  // Binary file constructor.
  trans_unit_context(const byte_stream_t& bytes);

public:
  // Relative path of the target file.
  const std::string file_path;
  // Plain text source of the target file.
  const std::string file_source;

  // Optimization level: 0-3
  size_t optimization_level = 0;

  std::unique_ptr<token_stream> tokens;
  std::unique_ptr<syntax_tree> ast;
  std::unique_ptr<bytecode_holder> bytecode;
  std::unique_ptr<constant_holder> constants;

  struct {
    size_t label_count;

    std::unique_ptr<compiler_variable_stack> variable_stack;
    std::unique_ptr<compiler_function_stack> function_stack;
    std::unique_ptr<global_holder> globals;
  } internal;
};

class compiler_context final {
public:
public:
  uint32_t flags;

  std::vector<trans_unit_context> units;
};

} // namespace via

#endif
