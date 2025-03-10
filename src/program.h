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
    U8 optimization_level = 0;

    std::string file;
    std::string source;

    TokenHolder*        tokens   = nullptr;
    AbstractSyntaxTree* ast      = nullptr;
    BytecodeHolder*     bytecode = nullptr;
    ConstantHolder*     constants;
    CompilerStack*      test_stack;
    GlobalTracker*      globals;

    VIA_CUSTOM_DESTRUCTOR(ProgramData);

    ProgramData(std::string file, std::string source);
};

VIA_NAMESPACE_END

#endif
