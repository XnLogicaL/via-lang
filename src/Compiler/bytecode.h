/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "instruction.h"
#include "Parser/ast.h"

namespace via::Compilation
{

class Bytecode
{
public:
    Bytecode() = default;
    ~Bytecode() = default;

    using Instructions_t = std::vector<viaInstruction>;
    Instructions_t instructions;
    Parsing::AST::AST *ast;

    void add_instruction(const viaInstruction &);
    void remove_instruction(size_t index);
    Instructions_t &get();
    const Instructions_t &get() const;
};

} // namespace via::Compilation