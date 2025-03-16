// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_PROGRAM_H
#define _VIA_PROGRAM_H

#include "globals.h"
#include "common_nodep.h"
#include "api_config.h"

VIA_NAMESPACE_BEGIN

struct TokenHolder;
struct AbstractSyntaxTree;
class BytecodeHolder;
class ConstantHolder;
class CompilerStack;

class ProgramData {
public:
    u8 optimization_level = 0;

    size_t label_count = 0;

    std::string file;
    std::string source;

    TokenHolder*        tokens   = nullptr;
    AbstractSyntaxTree* ast      = nullptr;
    BytecodeHolder*     bytecode = nullptr;
    ConstantHolder*     constants;
    CompilerStack*      test_stack;
    GlobalTracker*      globals;

    VIA_NON_COPYABLE(ProgramData);
    VIA_CUSTOM_DESTRUCTOR(ProgramData);

    // Move constructor
    ProgramData(ProgramData&& other) noexcept
        : optimization_level(other.optimization_level),
          label_count(other.label_count),
          file(other.file),
          source(other.source),
          tokens(other.tokens),
          ast(other.ast),
          bytecode(other.bytecode),
          constants(other.constants),
          test_stack(other.test_stack),
          globals(other.globals) {
        other.optimization_level = 0;
        other.label_count        = 0;
        other.file               = "";
        other.source             = "";

        other.tokens     = nullptr;
        other.ast        = nullptr;
        other.bytecode   = nullptr;
        other.constants  = nullptr;
        other.test_stack = nullptr;
        other.globals    = nullptr;
    }

    // Move assignment operator
    ProgramData& operator=(ProgramData&& other) noexcept = delete;

    ProgramData(std::string file, std::string source);
};

VIA_NAMESPACE_END

#endif
