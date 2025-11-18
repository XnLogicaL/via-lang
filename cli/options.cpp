/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "options.hpp"
#include <numeric>

void via::ProgramOptions::register_to(ArgumentParser& parser)
{}

std::string via::ProgramOptions::to_string() const
{
    return std::format(
        "ProgramOptions:\n"
        "  verbosity:   {}\n"
        "  no_execute:  {}\n"
        "  debugger:    {}\n"
        "  input:       {}\n"
        "  dump:        [{}]\n"
        "  imports:     [{}]",
        verbosity,
        no_execute,
        debugger,
        input.string(),
        dump.empty() ? ""
                     : std::accumulate(
                           std::next(dump.begin()),
                           dump.end(),
                           *dump.begin(),
                           [](std::string a, const std::string& b) {
                               return std::move(a) + ", " + b;
                           }
                       ),
        imports.empty() ? ""
                        : std::accumulate(
                              std::next(imports.begin()),
                              imports.end(),
                              *imports.begin(),
                              [](std::string a, const std::string& b) {
                                  return std::move(a) + ", " + b;
                              }
                          )
    );
}
