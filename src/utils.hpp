#pragma once

#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <typeinfo>
#include <typeindex>

template <typename... Ts>
std::type_index get_variant_type(const std::variant<Ts...> &v)
{
    return std::visit([](const auto &val) -> std::type_index
    { return typeid(val); }, v);
}

struct Console
{

    static void info(std::string msg)
    {
        std::cout << "[INFO] " << msg << std::endl;
    }

    static void warning(std::string msg)
    {
        std::cout << "[WARNING] " << msg << std::endl;
    }

    static void error(std::string msg)
    {
        std::cerr << "[ERROR] " << msg << std::endl;
    }

};

namespace UUID
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