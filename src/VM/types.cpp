#include "types.h"
#include "common.h"

namespace via {
namespace VM {

std::size_t via_TableKeyHash::operator()(const via_TableKey& key) const
{
    if (key.type == via_TableKey::KType::Number) {
        return std::hash<via_Number>()(key.num_val);
    } 
    else {
        return std::hash<std::string_view>()(key.str_val);
    }
}

bool via_TableKeyEqual::operator()(const via_TableKey& lhs, const via_TableKey& rhs) const
{
    if (lhs.type != rhs.type) return false;
    if (lhs.type == via_TableKey::KType::Number) {
        return lhs.num_val == rhs.num_val;
    }
    else {
        return strcmp(lhs.str_val, rhs.str_val) == 0;
    }
}

via_Value& via_Table::get(const via_TableKey& key)
{
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    static via_Value nil;
    return nil;
}

void via_Table::set(const via_TableKey& key, const via_Value& val)
{
    if (val.type == via_Value::VType::Nil)
    {
        if (get(key).type != via_Value::VType::Nil)
            data.erase(key);
        return;
    }
    data[key] = val;
}

via_Value& via_Value::operator=(const via_Value& other)
{
    if (this != &other)
    {
        cleanup();  // Clean up existing resources
        type = other.type;
        switch (type)
        {
        case VType::Number:
            num_val = other.num_val;
            break;
        case VType::Bool:
            bool_val = other.bool_val;
            break;
        case VType::String:
            str_val = strdup(other.str_val);  // Deep copy of the string
            break;
        case VType::Ptr:
            ptr_val = other.ptr_val;
            break;
        case VType::Func:
            fun_val = new via_Func(*other.fun_val); // Deep copy the function pointer
            break;
        case VType::CFunc:
            cfun_val = other.cfun_val;
            break;
        case VType::Table:
            tbl_val = new via_Table(*other.tbl_val); // Deep copy the table
            break;
        case VType::Nil:
            nil_val = nullptr;
            break;
        case VType::TableKey:
            tblkey_val = new via_TableKey(*other.tblkey_val); // Deep copy the table key
            break;
        default:
            break;
        }
    }
    return *this;
}

via_Value::via_Value(const via_Table& t)
{
    type = VType::Table;
    tbl_val = new via_Table(t);
}

via_Value::via_Value(const via_Value& other) : type(other.type)
{
    *this = other; // Assign to utilize operator=
}

void via_Value::cleanup()
{
    switch (type)
    {
    case VType::String:
        if (str_val) {
            free(str_val);
            str_val = nullptr;
        }
        break;
    case VType::Table:
        if (tbl_val) {
            delete tbl_val;
            tbl_val = nullptr;
        }
        break;
    case VType::Func:
        if (fun_val) {
            delete fun_val;
            fun_val = nullptr;
        }
        break;
    case VType::TableKey:
        if (tblkey_val) {
            delete tblkey_val;
            tblkey_val = nullptr;
        }
        break;
    default:
        break;
    }
}

} // namespace VM

} // namespace via
