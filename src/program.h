// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_PROGRAM_H
#define _VIA_PROGRAM_H

#include "globals.h"
#include "common-defs.h"
#include "common-macros.h"

#define VFLAG_VERBOSE int(1 << 0)
#define VFLAG_SASSY   int(1 << 7)

VIA_NAMESPACE_BEGIN

class TokenStream;
class AbstractSyntaxTree;
class BytecodeHolder;
class ConstantHolder;
class CompilerStack;

class ProgramData final {
public:
    int     flags              = 0;
    uint8_t optimization_level = 0;
    size_t  label_count        = 0;

    std::string file;
    std::string source;

    TokenStream*        token_stream;
    AbstractSyntaxTree* ast;
    BytecodeHolder*     bytecode;
    ConstantHolder*     constants;
    CompilerStack*      test_stack;
    GlobalTracker*      globals;

    VIA_NON_COPYABLE(ProgramData);
    VIA_CUSTOM_DESTRUCTOR(ProgramData);

    // Move constructor
    ProgramData(ProgramData&& other)
        : optimization_level(other.optimization_level),
          label_count(other.label_count),
          file(other.file),
          source(other.source),
          token_stream(other.token_stream),
          ast(other.ast),
          bytecode(other.bytecode),
          constants(other.constants),
          test_stack(other.test_stack),
          globals(other.globals) {
        other.optimization_level = 0;
        other.label_count        = 0;
        other.file               = "";
        other.source             = "";

        other.token_stream = nullptr;
        other.ast          = nullptr;
        other.bytecode     = nullptr;
        other.constants    = nullptr;
        other.test_stack   = nullptr;
        other.globals      = nullptr;
    }

    ProgramData& operator=(ProgramData&& other) = delete;
    ProgramData(std::string file, std::string source);
};

VIA_NAMESPACE_END

#endif
