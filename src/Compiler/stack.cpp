// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#include "stack.h"

namespace via {

void TestStack::push(TestStackMember val)
{
    sbp[sp++] = val;
}

TestStackMember TestStack::pop()
{
    TestStackMember val = sbp[sp--];
    return val;
}

TestStackMember TestStack::top()
{
    return sbp[sp--];
}

size_t TestStack::size()
{
    return sp;
}

std::optional<TestStackMember> TestStack::at(size_t pos)
{
    if (pos > size()) {
        return std::nullopt;
    }

    return sbp[pos];
}

std::optional<U32> TestStack::find_symbol(const TestStackMember &member)
{
    for (TestStackMember *stk_id = sbp; stk_id < sbp + sp; stk_id++) {
        if (stk_id->symbol == member.symbol) {
            return stk_id - sbp;
        }
    }

    return std::nullopt;
}

} // namespace via