/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "stack.h"

namespace via::Compilation
{

using namespace via::Parsing;
using namespace AST;

void TestStack::push_frame()
{
    stack.push(StackFrame());
}

void TestStack::pop_frame()
{
    stack.pop();
}

std::optional<ExprNode> TestStack::get_variable(std::string)
{
    return std::nullopt;
}


std::optional<ExprNode> TestStack::get_constant(std::string)
{
    return std::nullopt;
}

void declare_variable(std::string, Parsing::AST::ExprNode) {}
void declare_constant(std::string, Parsing::AST::ExprNode) {}

} // namespace via::Compilation