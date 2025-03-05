// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "globals.h"
#include "common_nodep.h"

namespace via {

struct TokenHolder;
struct AbstractSyntaxTree;
class BytecodeHolder;
class ConstantHolder;
class TestStack;

struct ProgramData {
    U8                  optimization_level = 0;
    std::string         file;
    std::string         source;
    TokenHolder        *tokens   = nullptr;
    AbstractSyntaxTree *ast      = nullptr;
    BytecodeHolder     *bytecode = nullptr;
    ConstantHolder     *constants;
    TestStack          *test_stack;
    GlobalTracker      *globals;

    ProgramData(std::string file, std::string source);
    ~ProgramData();
};

} // namespace via