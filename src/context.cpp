//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "common.h"
#include "ast.h"
#include "token.h"
#include "bytecode.h"
#include "constant.h"
#include "stack.h"

namespace via {

// ==========================================================================================
// trans_unit_context
trans_unit_context::trans_unit_context(const std::string& file_path, const std::string& file_source)
  : file_path(file_path),
    file_source(file_source),
    tokens(std::make_unique<token_stream>()),
    ast(std::make_unique<syntax_tree>()),
    bytecode(std::make_unique<bytecode_holder>()),
    constants(std::make_unique<constant_holder>()),
    internal({
      0,
      std::make_unique<compiler_variable_stack>(),
      std::make_unique<compiler_function_stack>(),
      std::make_unique<global_holder>(),
    }) {}

trans_unit_context::trans_unit_context(const byte_stream_t&) {}

void trans_unit_context::clear() {}

const char* trans_unit_context::get_platform_info() {
  static char buffer[32];
  static bool fetched = false;

#ifdef _WIN32
  const char* os = "windows";
#elifdef __linux__
  const char* os = "linux";
#else
  const char* os = "other";
#endif

#ifdef __x86_64__
  const char* arch = "x86-64";
#elifdef i386
  const char* arch = "x86-32";
#elifdef __aarch64__
  const char* arch = "arm-64";
#else
  const char* arch = "other";
#endif

  if (!fetched) {
    fetched = true;
    std::snprintf(buffer, sizeof(buffer), "%s-%s", os, arch);
  }

  return buffer;
}

byte_stream_t trans_unit_context::encode() {
  return {};
}

// ==========================================================================================
// compiler_context

} // namespace via
