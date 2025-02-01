/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "stack.h"

namespace via
{

TStack::TStack()
    : sp(0)
{
    void *mem = std::malloc(VIA_STACK_SIZE);
    this->sbp = reinterpret_cast<TValue *>(mem);

    VIA_ASSERT_SILENT(mem, "TStack(): std::malloc failed");
}

TStack::~TStack()
{
    std::free(this->sbp);
}

} // namespace via