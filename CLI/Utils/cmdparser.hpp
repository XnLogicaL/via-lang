/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

namespace viaCLI
{

class CmdParser
{
public:
    CmdParser(int argc, char **argv)
        : argc(argc)
        , argv(argv)
    {
    }

    bool is_valid()
    {
        return argc >= 2;
    }

    std::string get_subcommand()
    {
        return argv[1];
    }

    std::vector<std::string> get_arguments()
    {
        return std::vector<std::string>(argv + 2, argv + argc);
    }

private:
    const int argc;
    char **const argv;
};

} // namespace viaCLI