// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "vlbase.h"
#include "api.h"
#include "vmapi.h"
#include "libutils.h"
#include "types.h"

namespace via::lib {

using namespace impl;

LIB_DECL_FUNCTION(base_print)
{
    U16 i = 0;
    std::ostringstream oss;

    while (i++ < V->argc) {
        LIB_DECL_PARAMETER(argx, i);
        oss << __to_cxx_string(V, argx) << " ";
    }

    std::cout << oss.str();

    LIB_RETURN(0)
}

LIB_DECL_FUNCTION(base_println)
{
    U8 i = 0;
    std::ostringstream oss;

    while (i++ < V->argc) {
        LIB_DECL_PARAMETER(argx, i);
        oss << __to_cxx_string(V, argx) << " ";
    }

    std::cout << oss.str() << "\n";

    LIB_RETURN(0);
}

LIB_DECL_FUNCTION(base_error)
{
    LIB_DECL_PARAMETER(arg0, 0);

    __set_error_state(V, __to_cxx_string(V, arg0));

    LIB_RETURN(0);
}

LIB_DECL_FUNCTION(base_assert)
{
    LIB_DECL_PARAMETER(arg0, 0);
    LIB_DECL_PARAMETER(arg1, 1);

    if (!__to_cxx_bool(arg0)) {
        std::string err_cxx_str = std::format("assertion failed: {}", __to_cxx_string(V, arg1));

        TValue err_val(new TString(V, err_cxx_str.c_str()));
        TValue err_fn = LIB_WRAP_CFPTR(base_error);

        __push(V, err_val.clone());
        __call(V, err_fn, 1);
    }

    LIB_RETURN(0);
}

LIB_DECL_FUNCTION(base_weakPrimCast)
{
    LIB_DECL_PARAMETER(val, 0);
    LIB_DECL_PARAMETER(type, 1);

    if (!check_string(type)) {
        LIB_ERR_ARG_TYPE_MISMATCH("string", __type_cxx_string(V, type), 1);
    }

    std::string type_str = __to_cxx_string(V, type);
    std::optional<ValueType> val_type = magic_enum::enum_cast<ValueType>(type_str);

    LIB_ASSERT(
        val_type.has_value(), std::format("'{}' is not a valid primitive typename", type_str)
    );

    TValue casted_val = __weak_primitive_cast(V, val, val_type.value());

    __push(V, casted_val.clone());
    LIB_RETURN(1);
}

LIB_DECL_FUNCTION(base_strongPrimCast)
{
    LIB_DECL_PARAMETER(val, 0);
    LIB_DECL_PARAMETER(type, 1);

    if (!check_string(type)) {
        LIB_ERR_ARG_TYPE_MISMATCH("string", __type_cxx_string(V, type), 1);
    }

    std::string type_str = __to_cxx_string(V, type);
    std::optional<ValueType> val_type = magic_enum::enum_cast<ValueType>(type_str);

    LIB_ASSERT(
        val_type.has_value(), std::format("'{}' is not a valid primitive typename", type_str)
    );

    __strong_primtive_cast(V, const_cast<TValue &>(val), val_type.value());
    LIB_RETURN(0);
}

LIB_DECL_FUNCTION(open_baselib)
{
    std::unordered_map<U32, TValue> base_properties;

    LIB_MAP_EMPLACE(base_properties, "print", LIB_WRAP_CFPTR(base_print));
    LIB_MAP_EMPLACE(base_properties, "println", LIB_WRAP_CFPTR(base_println));
    LIB_MAP_EMPLACE(base_properties, "error", LIB_WRAP_CFPTR(base_error));
    LIB_MAP_EMPLACE(base_properties, "assert", LIB_WRAP_CFPTR(base_assert));
    LIB_MAP_EMPLACE(base_properties, "weakPrimitiveCast", LIB_WRAP_CFPTR(base_weakPrimCast));
    LIB_MAP_EMPLACE(base_properties, "strongPrimitiveCast", LIB_WRAP_CFPTR(base_strongPrimCast));

    for (const auto &[ident, val] : base_properties) {
        __set_global(V, ident, val);
    }
}

} // namespace via::lib
