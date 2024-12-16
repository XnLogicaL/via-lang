/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "Parser/ast.h"

namespace via::Compilation
{

class TestStack
{
public:
    struct StackFrame
    {
        std::unordered_map<std::string, Parsing::AST::ExprNode> variables;
        std::unordered_map<std::string, Parsing::AST::ExprNode> constants;
    };

    TestStack() = default;
    ~TestStack() = default;

    void push_frame();
    void pop_frame();
    std::optional<Parsing::AST::ExprNode> get_variable(std::string);
    std::optional<Parsing::AST::ExprNode> get_constant(std::string);
    void declare_variable(std::string, Parsing::AST::ExprNode);
    void declare_constant(std::string, Parsing::AST::ExprNode);

private:
    std::stack<StackFrame> stack;
};

} // namespace via::Compilation