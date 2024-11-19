/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "types.h"

namespace via::VM
{

size_t via_Table::via_TableKeyHash::operator()(const via_TableKey &key) const
{
    if (key.type == via_TableKey::KType::Number)
    {
        return std::hash<via_Number>()(key.num);
    }
    else
    {
        return std::hash<std::string_view>()(key.str);
    }
}

bool via_Table::via_TableKeyEqual::operator()(const via_TableKey &lhs, const via_TableKey &rhs) const
{
    if (lhs.type != rhs.type)
        return false;
    if (lhs.type == via_TableKey::KType::Number)
    {
        return lhs.num == rhs.num;
    }
    else
    {
        return strcmp(lhs.str, rhs.str) == 0;
    }
}

via_Value &via_Table::get(const via_TableKey &key)
{
    auto it = data.find(key);
    if (it != data.end())
    {
        return it->second;
    }
    static via_Value nil;
    return nil;
}

void via_Table::set(const via_TableKey &key, const via_Value &val)
{
    if (val.type == ValueType::Nil)
    {
        if (get(key).type != ValueType::Nil)
            data.erase(key);
        return;
    }
    data[key] = val;
}

via_Value &via_Value::operator=(const via_Value &other)
{
    if (this != &other)
    {
        // cleanup(); // Clean up existing resources
        type = other.type;
        switch (type)
        {
        case VType::Number:
            num = other.num;
            break;
        case VType::Bool:
            boole = other.boole;
            break;
        case VType::String:
            str = strdup(other.str); // Deep copy of the string
            break;
        case VType::Ptr:
            ptr = other.ptr;
            break;
        case VType::Func:
            fun = new via_Func(*other.fun); // Deep copy the function pointer
            break;
        case VType::CFunc:
            cfun = other.cfun;
            break;
        case VType::Table:
            tbl = new via_Table(*other.tbl); // Deep copy the table
            break;
        case VType::Nil:
            nil = nullptr;
            break;
        case VType::TableKey:
            tblkey = new via_TableKey(*other.tblkey); // Deep copy the table key
            break;
        default:
            break;
        }
    }
    return *this;
}

via_Value::via_Value(const via_Table &t)
{
    type = VType::Table;
    tbl = new via_Table(t);
}

via_Value::via_Value(const via_Value &other)
    : type(other.type)
{
    *this = other; // Assign to utilize operator=
}

void via_Value::cleanup()
{
    switch (type)
    {
    case VType::String:
        if (str)
        {
            std::free(str);
            str = nullptr;
        }
        break;
    case VType::Table:
        if (tbl)
        {
            delete tbl;
            tbl = nullptr;
        }
        break;
    case VType::Func:
        if (fun)
        {
            delete fun;
            fun = nullptr;
        }
        break;
    case VType::TableKey:
        if (tblkey)
        {
            delete tblkey;
            tblkey = nullptr;
        }
        break;
    default:
        break;
    }
}

} // namespace via::VM
