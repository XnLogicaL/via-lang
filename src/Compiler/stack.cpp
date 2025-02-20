// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#include "stack.h"

namespace via {

void TestStack::push(std::string val)
{
    sbp[sp++] = val;
}

std::string TestStack::pop()
{
    std::string val = sbp[sp--];
    return val;
}

std::string TestStack::top()
{
    return sbp[sp--];
}

size_t TestStack::size()
{
    return sp;
}

std::optional<std::string> TestStack::at(size_t pos)
{
    if (pos > size())
        return std::nullopt;

    return sbp[pos];
}

size_t TestStack::find(const std::string &val)
{
    for (size_t i = 0; i < size(); i++) {
        std::optional<std::string> val_at = at(i);
        if (val_at.has_value() && val_at->compare(val) == 0)
            return i;
    }

    return SIZE_MAX;
}

} // namespace via