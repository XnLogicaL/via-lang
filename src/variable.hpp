#pragma once

#include <iostream>
#include <variant>
#include <string>
#include <sstream>

// Define Type and Value as variants
typedef std::variant<std::string, int> Type;
typedef std::variant<int, float, char> Value;

class Variable
{
public:
    bool is_global;
    bool is_const;

    Type type;
    Value value;
    std::string id;

    Variable(bool v_is_global, bool v_is_const, Type v_type, Value v_value = Value{}, std::string v_identifier = "")
        : is_global(v_is_global), is_const(v_is_const), type(v_type), value(v_value), id(v_identifier) {}

    template <typename T>
    std::string variant_to_string(const T &var) const
    {
        return std::visit(
            [](const auto &val)->std::string
            {
                std::ostringstream oss;
                oss << val;
                return oss.str();
            },
            var
        );
    }
};

// Define the output stream operator outside of the class
std::ostream &operator<<(std::ostream &os, const Variable &obj)
{
    os << "Variable(IsConst: " << std::boolalpha << obj.is_const
       << ",  IsGlobal: " << std::boolalpha << obj.is_global
       << ",  Value: " << obj.variant_to_string(obj.value)
       << ",  Type: " << obj.variant_to_string(obj.type)
       << ",  ID: " << obj.id
       << ")";
    return os;
}