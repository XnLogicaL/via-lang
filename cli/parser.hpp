/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <span>
#include <via/via.hpp>

namespace via {

class ArgumentParser final
{
  public:
    explicit ArgumentParser(int argc, char** argv)
        : args((const char**) argv, (size_t) argc)
    {}

  public:
    bool parse_args();

  private:
    std::span<const char*> args;
};

} // namespace via
