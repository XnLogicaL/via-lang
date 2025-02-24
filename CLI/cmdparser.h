// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"

namespace via {

class CmdParser {
public:
    CmdParser(int argc, char **argv)
        : argc(argc)
        , argv(argv)
    {
    }

    inline bool is_valid()
    {
        return argc >= 2;
    }

    inline std::string get_subcommand()
    {
        return argv[1];
    }

    inline std::vector<std::string> get_arguments()
    {
        return std::vector<std::string>(argv + 2, argv + argc);
    }

private:
    const int argc;
    char **const argv;
};

} // namespace via