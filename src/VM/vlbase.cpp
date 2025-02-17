/* This file is a part of the via programming language at
 * https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

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

        __push(V, err_val);
        __call(V, err_fn, 1);
    }

    LIB_RETURN(0);
}

LIB_DECL_FUNCTION(base_weak_prim_cast)
{
    LIB_DECL_PARAMETER(val, 0);
    LIB_DECL_PARAMETER(type, 1);

    if (!check_string(type)) {
        LIB_ERR_ARG_TYPE_MISMATCH("string", __type_cxx_string(V, type), 1);
    }

    std::string type_str = __to_cxx_string(V, type);
    std::optional<ValueType> val_type = ENUM_CAST(ValueType, type_str);

    LIB_ASSERT(val_type.has_value(), std::format("'{}' is not a valid primitive typename", type_str));

    TValue casted_val = __weak_primitive_cast(V, val, val_type.value());

    __push(V, casted_val);
    LIB_RETURN(1);
}

LIB_DECL_FUNCTION(base_strong_prim_cast)
{
    LIB_DECL_PARAMETER(val, 0);
    LIB_DECL_PARAMETER(type, 1);

    if (!check_string(type)) {
        LIB_ERR_ARG_TYPE_MISMATCH("string", __type_cxx_string(V, type), 1);
    }

    std::string type_str = __to_cxx_string(V, type);
    std::optional<ValueType> val_type = ENUM_CAST(ValueType, type_str);

    LIB_ASSERT(val_type.has_value(), std::format("'{}' is not a valid primitive typename", type_str));

    // Sorry about this...
    __strong_primtive_cast(V, const_cast<TValue &>(val), val_type.value());
    LIB_RETURN(0);
}

LIB_DECL_FUNCTION(open_baselib)
{
    std::unordered_map<kGlobId, TValue> base_properties;

    base_properties.emplace("print", LIB_WRAP_CFPTR(base_print));
    base_properties.emplace("println", LIB_WRAP_CFPTR(base_println));
    base_properties.emplace("error", LIB_WRAP_CFPTR(base_error));
    base_properties.emplace("assert", LIB_WRAP_CFPTR(base_assert));
    base_properties.emplace("weakPrimitiveCast", LIB_WRAP_CFPTR(base_weak_prim_cast));
    base_properties.emplace("strongPrimitiveCast", LIB_WRAP_CFPTR(base_strong_prim_cast));

    for (const auto &[ident, val] : base_properties) {
        __set_global(V, ident, val);
    }
}

} // namespace via::lib
