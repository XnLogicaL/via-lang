// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "stack.h"

VIA_NAMESPACE_BEGIN

void TestStack::push(TestStackMember val) {
    sbp[sp++] = val;
}

TestStackMember TestStack::pop() {
    TestStackMember val = sbp[sp--];
    return val;
}

TestStackMember TestStack::top() {
    return sbp[sp--];
}

U64 TestStack::size() {
    return sp;
}

std::optional<TestStackMember> TestStack::at(SIZE pos) {
    if (pos > size()) {
        return std::nullopt;
    }

    return sbp[pos];
}

std::optional<Operand> TestStack::find_symbol(const TestStackMember& member) {
    for (TestStackMember* stk_id = sbp; stk_id < sbp + sp; stk_id++) {
        if (stk_id->symbol == member.symbol) {
            return stk_id - sbp;
        }
    }

    return std::nullopt;
}

VIA_NAMESPACE_END
