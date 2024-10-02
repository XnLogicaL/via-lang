#pragma once

#ifndef via_common_hpp
#define via_common_hpp

// C Libs
#include <cstdbool>
#include <cstddef>
#include <cstdint>
#include <cstdio>

// C++ Libs
#include <string>
#include <optional>
#include <variant>
#include <vector>
#include <iostream>
#include <algorithm>
#include <memory>

// External libs
#include "../../include/magic_enum/magic_enum.hpp"
#include "../../include/arena_allocator.hpp"

#include "Utils/color.hpp"
#include "Utils/timer.hpp"
#include "Utils/uuidgen.hpp"

// Definitions
namespace CompilerOut
{
    void error(const std::string &msg)
    {
        std::cerr << Dye::style_text(
            Dye::TextColor::RED,
            Dye::BackgroundColor::NONE,
            Dye::TextStyle::NONE,
            "error: "
        ) << msg << "\n\nCompilation aborted" << std::endl;

        exit(1);
    }

    void warn(const std::string& msg)
    {
        std::cout << Dye::style_text(
            Dye::TextColor::YELLOW,
            Dye::BackgroundColor::NONE,
            Dye::TextStyle::NONE,
            "warning: "
        ) << msg << std::endl;
    }
    
    void info(const std::string& msg)
    {
        std::cout << Dye::style_text(
            Dye::TextColor::BLUE,
            Dye::BackgroundColor::NONE,
            Dye::TextStyle::NONE,
            "info: "
        ) << msg << std::endl;
    }

} // namespace CompilerOut

namespace VMOut
{
    
} // namespace VMOut


#endif