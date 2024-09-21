#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdlib>

class Command
{
public:

    int argc;
    char** argv;

    Command(auto _Ac, auto _Av)
        : argc(_Ac), argv(_Av) {}

    auto get_flags()
    {
        std::vector<std::string> flags = {};

        for (int i = 0; i < argc; i++)
        {
            auto arg_string = std::string(argv[i]);

            if (arg_string.starts_with("--") || arg_string.starts_with("-"))
                flags.push_back(arg_string);
        }

        return flags;
    }

    bool has_flag(std::string flag)
    {
        auto flags = get_flags();
        return std::find(flags.begin(), flags.end(), flag) != flags.end();
    }

};