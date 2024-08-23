#pragma once

#include <vector>
#include <variant>

#include "lexer.hpp"

class MathExpression;

typedef std::variant<MathExpression *, Token> MathVal;

class MathExpression
{
    MathVal left;
    MathVal right;

    Token math_op;

    MathExpression(MathVal left, MathVal right, Token math_op)
        : left(left), right(right), math_op(math_op) {}
};