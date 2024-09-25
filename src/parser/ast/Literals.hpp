#pragma once

#include "../../lexer/token.hpp"

struct IntLitNode {
    Token val;

    bool is_truthy() {return this->val.value != "0";}
};
struct BoolLitNode {
    Token val;

    bool is_truthy() {return this->val.value == "true";}
};
struct StringLitNode {
    Token val;
    bool is_truthy() {return true;}
};
struct IdentNode {
    Token val;
};
struct NilNode { 
    Token val = NULL_TOKEN;
    bool is_truthy() {return false;}
};
struct ParamNode { Token ident; Token type; };