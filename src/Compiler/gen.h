/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"

static unsigned int _iota = std::numeric_limits<unsigned int>::max();

namespace via::Compilation
{

unsigned int iota()
{
    return _iota--;
}

class Generator
{
public:

    void push(const std::string &line);
    void pushline(const std::string &line);

    std::string finalize();

    size_t get_available_register();
    void free_register(size_t offset);
private:

    std::string src;
    std::vector<bool> registers;
};

} // namespace via::Compilation
