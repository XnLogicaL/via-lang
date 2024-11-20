/* This file is a part of the via programming language at
 * https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "types.h"

namespace via::VM
{

size_t viaTable::__hash::operator()(const viaTableKey &key) const
{
    if (key.type == viaTableKey::__type::viaNumber)
        return std::hash<viaNumber>()(key.num);
    else
        return std::hash<std::string_view>()(key.str);
}

bool viaTable::__eq::operator()(const viaTableKey &lhs, const viaTableKey &rhs) const
{
    if (lhs.type != rhs.type)
        return false;
    if (lhs.type == viaTableKey::__type::viaNumber)
        return lhs.num == rhs.num;

    return strcmp(lhs.str, rhs.str) == 0;
}

viaValue &viaValue::operator=(const viaValue &other)
{
    if (this != &other)
    {
        // cleanup(); // Clean up existing resources
        type = other.type;
        switch (type)
        {
        case __type::viaNumber:
            num = other.num;
            break;
        case __type::Bool:
            boole = other.boole;
            break;
        case __type::String:
            str = strdup(other.str); // Deep copy of the string
            break;
        case __type::Ptr:
            ptr = other.ptr;
            break;
        case __type::Func:
            fun = new Func(*other.fun); // Deep copy the function pointer
            break;
        case __type::CFunc:
            cfun = other.cfun;
            break;
        case __type::viaTable:
            tbl = new viaTable(*other.tbl); // Deep copy the table
            break;
        case __type::Nil:
            nil = nullptr;
            break;
        default:
            break;
        }
    }
    return *this;
}

viaValue::viaValue(const viaTable &t)
{
    type = __type::viaTable;
    tbl = new viaTable(t);
}

viaValue::viaValue(const viaValue &other)
    : type(other.type)
{
    *this = other; // Assign to utilize operator=
}

void viaValue::cleanup()
{
    switch (type)
    {
    case __type::String:
        if (str)
        {
            std::free(str);
            str = nullptr;
        }
        break;
    case __type::viaTable:
        if (tbl)
        {
            delete tbl;
            tbl = nullptr;
        }
        break;
    case __type::Func:
        if (fun)
        {
            delete fun;
            fun = nullptr;
        }
        break;
    default:
        break;
    }
}

} // namespace via::VM
