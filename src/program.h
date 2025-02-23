// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
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
    uint8_t optimization_level = 0;
    std::string file;
    std::string source;
    TokenHolder *tokens = nullptr;
    AbstractSyntaxTree *ast = nullptr;
    BytecodeHolder *bytecode = nullptr;
    ConstantHolder *constants;
    TestStack *test_stack;
    GlobalTracker *globals;
    std::map<size_t, std::string> bytecode_info;

    ProgramData(std::string file, std::string source);
    ~ProgramData();
};

} // namespace via