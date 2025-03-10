// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "vlbase.h"
#include "api.h"
#include "vmapi.h"
#include "libutils.h"
#include "rttypes.h"

VIA_NAMESPACE_LIB_BEGIN

using namespace impl;

VIA_LIB_DECL_FUNCTION(base_print) {
    std::ostringstream oss;

    U16 i = 0;
    while (i++ < V->argc) {
        VIA_LIB_DECL_PARAMETER(argx, i);
        oss << __to_cxx_string(V, argx) << " ";
    }

    std::cout << oss.str();

    VIA_LIB_RETURN(0)
}

VIA_LIB_DECL_FUNCTION(base_println) {
    std::ostringstream oss;

    U8 i = 0;
    while (i++ < V->argc) {
        VIA_LIB_DECL_PARAMETER(argx, i);
        oss << __to_cxx_string(V, argx) << " ";
    }

    std::cout << oss.str() << "\n";

    VIA_LIB_RETURN(0);
}

VIA_LIB_DECL_FUNCTION(base_error) {
    VIA_LIB_DECL_PARAMETER(arg0, 0);
    __set_error_state(V, __to_cxx_string(V, arg0));
    VIA_LIB_RETURN(0);
}

VIA_LIB_DECL_FUNCTION(base_assert) {
    VIA_LIB_DECL_PARAMETER(arg0, 0);
    VIA_LIB_DECL_PARAMETER(arg1, 1);

    if (!__to_cxx_bool(arg0)) {
        std::string err_cxx_str = std::format("assertion failed: {}", __to_cxx_string(V, arg1));

        TValue err_val(new TString(V, err_cxx_str.c_str()));
        TValue err_fn = VIA_LIB_WRAP_CFPTR(base_error);

        __push(V, err_val.clone());
        __call(V, err_fn, 1);
    }

    VIA_LIB_RETURN(0);
}

VIA_LIB_DECL_FUNCTION(base_weakPrimCast) {
    VIA_LIB_DECL_PARAMETER(val, 0);
    VIA_LIB_DECL_PARAMETER(type, 1);

    std::string              type_str = __to_cxx_string(V, type);
    std::optional<ValueType> val_type = magic_enum::enum_cast<ValueType>(type_str);

    VIA_LIB_ASSERT(
        val_type.has_value(), std::format("'{}' is not a valid primitive typename", type_str)
    );

    TValue casted_val = __weak_primitive_cast(V, val, val_type.value());

    __push(V, casted_val.clone());
    VIA_LIB_RETURN(1);
}

VIA_LIB_DECL_FUNCTION(base_strongPrimCast) {
    VIA_LIB_DECL_PARAMETER(val, 0);
    VIA_LIB_DECL_PARAMETER(type, 1);

    std::string              type_str = __to_cxx_string(V, type);
    std::optional<ValueType> val_type = magic_enum::enum_cast<ValueType>(type_str);

    VIA_LIB_ASSERT(
        val_type.has_value(), std::format("'{}' is not a valid primitive typename", type_str)
    );

    __strong_primitive_cast(V, const_cast<TValue&>(val), val_type.value());
    VIA_LIB_RETURN(0);
}

VIA_LIB_DECL_FUNCTION(open_baselib) {
    std::unordered_map<U32, TValue> base_properties;

    VIA_LIB_MAP_EMPLACE(base_properties, "print", VIA_LIB_WRAP_CFPTR(base_print));
    VIA_LIB_MAP_EMPLACE(base_properties, "println", VIA_LIB_WRAP_CFPTR(base_println));
    VIA_LIB_MAP_EMPLACE(base_properties, "error", VIA_LIB_WRAP_CFPTR(base_error));
    VIA_LIB_MAP_EMPLACE(base_properties, "assert", VIA_LIB_WRAP_CFPTR(base_assert));
    VIA_LIB_MAP_EMPLACE(
        base_properties, "weakPrimitiveCast", VIA_LIB_WRAP_CFPTR(base_weakPrimCast)
    );
    VIA_LIB_MAP_EMPLACE(
        base_properties, "strongPrimitiveCast", VIA_LIB_WRAP_CFPTR(base_strongPrimCast)
    );

    for (const auto& [ident, val] : base_properties) {
        __set_global(V, ident, val);
    }
}

VIA_NAMESPACE_END
