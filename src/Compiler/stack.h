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
public:
    TestStack()
        : sbp(reinterpret_cast<std::string *>(std::malloc(VIA_TEST_STACK_SIZE)))
        , sp(0)
    {
    }

    void push(std::string);
    std::string top();
    std::string pop();
    size_t size();
    std::optional<std::string> at(size_t);
    size_t find(const std::string &);

private:
    std::string *sbp;
    size_t sp;
};

} // namespace via