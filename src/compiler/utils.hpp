#pragma once

#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <typeinfo>
#include <typeindex>
#include <variant>
#include <cstdint>

#include "../include/color.hpp"

namespace Console
{

    void CompilerError(std::string message)
    {
        std::cerr << dye::red("Compile error") << ": " << message << std::endl;
    }

    void CompilerInfo(std::string message)
    {
        std::cout << dye::aqua("Compile info") << ": " << message << std::endl;
    }

    void CompilerWarning(std::string message)
    {
        std::cout << dye::red_on_yellow("Compile warning") << ": " << message << std::endl;
    }

};

namespace UUID_
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    std::string generate_uuid_v4()
    {
        std::stringstream ss;
        int i;
        ss << std::hex;
        for (i = 0; i < 8; i++)
        {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 4; i++)
        {
            ss << dis(gen);
        }
        ss << "-4";
        for (i = 0; i < 3; i++)
        {
            ss << dis(gen);
        }
        ss << "-";
        ss << dis2(gen);
        for (i = 0; i < 3; i++)
        {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 12; i++)
        {
            ss << dis(gen);
        };
        return ss.str();
    }
}

auto deref_mem_address(uintptr_t p)
{
    return *reinterpret_cast<int *>(p);
}