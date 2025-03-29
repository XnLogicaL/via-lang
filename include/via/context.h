// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_CONTEXT_H
#define _VIA_CONTEXT_H

#include "common-defs.h"
#include "common-macros.h"
#include "compiler/globals.h"
#include "lex/token.h"

#define VFLAG_VERBOSE int(1 << 0)
#define VFLAG_SASSY   int(1 << 7)

VIA_NAMESPACE_BEGIN

using ByteStream = std::vector<uint8_t>;

class TokenStream;
class SyntaxTree;
class BytecodeHolder;
class ConstantHolder;
class CompilerStack;
class GlobalTracker;

class TransUnitContext final {
public:
  // Resets the translation unit context.
  void clear();

  // Encodes the translation unit onto a byte stream.
  ByteStream encode();

  const char* get_platform_info();

  TransUnitContext(const std::string& file_path, const std::string& file_source);
  TransUnitContext(const ByteStream& bytes);

public:
  const std::string file_path;
  const std::string file_source;

  std::unique_ptr<TokenStream> tokens;
  std::unique_ptr<SyntaxTree> ast;
  std::unique_ptr<BytecodeHolder> bytecode;
  std::unique_ptr<ConstantHolder> constants;

  struct Internal {
    size_t label_count;

    std::unique_ptr<CompilerStack> stack;
    std::unique_ptr<GlobalTracker> globals;
  } internal;
};

class Context final {
public:
public:
  uint32_t flags;

  std::vector<TransUnitContext> units;
};

VIA_NAMESPACE_END

#endif
