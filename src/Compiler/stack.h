/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "types.h"
#include "common.h"

#ifndef VIA_TEST_STACK_SIZE
    #define VIA_TEST_STACK_SIZE 1024 * 1024 // 1 MB
#endif

namespace via
{

class TestStack
{
    using Ty = std::string;

public:
    TestStack()
        : sbp(reinterpret_cast<Ty *>(std::malloc(VIA_TEST_STACK_SIZE)))
        , sp(0)
    {
    }

    void push(Ty);
    Ty top();
    Ty pop();
    size_t size();
    std::optional<Ty> at(size_t);
    size_t find(const Ty &);

private:
    Ty *sbp;
    size_t sp;
};

} // namespace via