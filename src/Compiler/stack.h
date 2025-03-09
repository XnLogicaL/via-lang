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

struct TestStackMember {
    std::string symbol         = "<anonymous-symbol>";
    bool        is_const       = false;
    bool        is_constexpr   = false;
    ValueType   primitive_type = ValueType::nil;
};

class TestStack {
public:
    TestStack()
        : sbp(new TestStackMember[VIA_TEST_STACK_SIZE]) {}

    ~TestStack() {
        delete[] sbp;
    }

    void                           push(TestStackMember);
    TestStackMember                top();
    TestStackMember                pop();
    U64                            size();
    std::optional<TestStackMember> at(SIZE);
    std::optional<Operand>         find_symbol(const TestStackMember&);

public:
    U64                                 sp = 0;
    std::stack<FunctionNode::StackNode> function_stack;

private:
    TestStackMember* sbp = nullptr;
};

VIA_NAMESPACE_END

#endif
