// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "types.h"
#include "common.h"

#define VIA_TEST_STACK_SIZE 1024 * 1024 // 1 MB

namespace via {

struct TestStackMember {
    std::string symbol = "<anonymous-symbol>";
    bool is_const = false;
    bool is_constexpr = false;
};

class TestStack {
public:
    TestStack()
        : sbp(new TestStackMember[VIA_TEST_STACK_SIZE])
    {
    }

    ~TestStack()
    {
        delete[] sbp;
    }

    void push(TestStackMember);
    TestStackMember top();
    TestStackMember pop();
    size_t size();
    std::optional<TestStackMember> at(size_t);
    std::optional<U32> find_symbol(const TestStackMember &);

private:
    TestStackMember *sbp = nullptr;
    size_t sp = 0;
};

} // namespace via