/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "stack.h"

namespace via
{

void TestStack::push(TestStack::Ty val)
{
    sbp[sp++] = val;
}

TestStack::Ty TestStack::pop()
{
    Ty val = sbp[sp--];
    return val;
}

TestStack::Ty TestStack::top()
{
    return sbp[sp--];
}

size_t TestStack::size()
{
    return sp;
}

std::optional<TestStack::Ty> TestStack::at(size_t pos)
{
    if (pos > size())
        return std::nullopt;

    return sbp[pos];
}

size_t TestStack::find(const TestStack::Ty &val)
{
    for (size_t i = 0; i < size(); i++)
    {
        std::optional<Ty> val_at = at(i);
        if (val_at.has_value() && val_at->compare(val) == 0)
            return i;
    }

    return SIZE_MAX;
}

} // namespace via