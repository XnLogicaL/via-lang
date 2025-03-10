// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_STACK_H
#define _VIA_STACK_H

#include "ast.h"
#include "ast_base.h"
#include "rttypes.h"
#include "common.h"

#define VIA_TEST_STACK_SIZE 1024 * 1024 // 1 MB

VIA_NAMESPACE_BEGIN

struct StackObject {
    bool is_const     = false;
    bool is_constexpr = false;

    std::string symbol = "<anonymous-symbol>";

    pTypeNode type;
};

class CompilerStack {
public:
    CompilerStack()
        : sbp(new StackObject[VIA_TEST_STACK_SIZE]) {}

    ~CompilerStack() {
        delete[] sbp;
    }

    void push(StackObject);

    StackObject top();
    StackObject pop();

    U64 size();

    std::optional<StackObject> at(SIZE);

    std::optional<Operand> find_symbol(const StackObject&);
    std::optional<Operand> find_symbol(const std::string&);

public:
    U64 sp = 0;

    std::stack<FunctionNode::StackNode> function_stack;

private:
    StackObject* sbp = nullptr;
};

VIA_NAMESPACE_END

#endif
