// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "common.h"
#include "ast.h"
#include "token.h"
#include "bytecode.h"
#include "constant.h"
#include "stack.h"

VIA_NAMESPACE_BEGIN

// ==========================================================================================
// TransUnitContext
TransUnitContext::Internal::Internal()
    : stack(std::make_unique<CompilerStack>()),
      globals(std::make_unique<GlobalTracker>()) {}

TransUnitContext::TransUnitContext(const std::string& file_path, const std::string& file_source)
    : file_path(file_path),
      file_source(file_source),
      tokens(std::make_unique<TokenStream>()),
      ast(std::make_unique<SyntaxTree>()),
      bytecode(std::make_unique<BytecodeHolder>()),
      constants(std::make_unique<ConstantHolder>()),
      internal() {}

TransUnitContext::TransUnitContext(const ByteStream&) {}

void TransUnitContext::clear() {}

const char* TransUnitContext::get_platform_info() {
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

ByteStream TransUnitContext::encode() {
  return {};
}

// ==========================================================================================
// Context

VIA_NAMESPACE_END
