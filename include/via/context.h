// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_context_h
#define vl_has_header_context_h

#include "common-defs.h"
#include "common-macros.h"
#include "compiler/globals.h"
#include "lex/token.h"

#define vflag_verbose int(1 << 0)
#define vflag_sassy   int(1 << 7)

namespace via {

using byte_stream_t = std::vector<uint8_t>;

class token_stream;
class syntax_tree;
class bytecode_holder;
class constant_holder;
class variable_stack;
class function_stack;
class global_holder;

class trans_unit_context final {
public:
  // Resets the translation unit context.
  void clear();

  // Encodes the translation unit onto a byte stream.
  byte_stream_t encode();

  const char* get_platform_info();

  trans_unit_context(const std::string& file_path, const std::string& file_source);
  trans_unit_context(const byte_stream_t& bytes);

public:
  const std::string file_path;
  const std::string file_source;

  std::unique_ptr<token_stream> tokens;
  std::unique_ptr<syntax_tree> ast;
  std::unique_ptr<bytecode_holder> bytecode;
  std::unique_ptr<constant_holder> constants;

  struct Internal {
    size_t label_count;

    std::unique_ptr<variable_stack> variable_stack;
    std::unique_ptr<function_stack> function_stack;
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
