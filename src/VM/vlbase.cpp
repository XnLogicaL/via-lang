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

    u16 i = 0;
    while (i++ < V->argc) {
        VIA_LIB_DECL_PARAMETER(argx, i);
        oss << __to_cxx_string(V, argx) << " ";
    }

    std::cout << oss.str();

    VIA_LIB_RETURN(nil);
}

VIA_LIB_DECL_FUNCTION(base_println) {
    std::ostringstream oss;

    u8 i = 0;
    while (i++ < V->argc) {
        VIA_LIB_DECL_PARAMETER(argx, i);
        oss << __to_cxx_string(V, argx) << " ";
    }

    std::cout << oss.str() << "\n";

    VIA_LIB_RETURN(nil);
}

VIA_LIB_DECL_FUNCTION(base_error) {
    VIA_LIB_DECL_PARAMETER(arg0, 0);
    __set_error_state(V, __to_cxx_string(V, arg0));
    VIA_LIB_RETURN(nil);
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

    VIA_LIB_RETURN(nil);
}

VIA_LIB_DECL_FUNCTION(open_baselib) {
    std::unordered_map<u32, TValue> base_properties;

    VIA_LIB_MAP_EMPLACE(base_properties, "print", VIA_LIB_WRAP_CFPTR(base_print));
    VIA_LIB_MAP_EMPLACE(base_properties, "println", VIA_LIB_WRAP_CFPTR(base_println));
    VIA_LIB_MAP_EMPLACE(base_properties, "error", VIA_LIB_WRAP_CFPTR(base_error));
    VIA_LIB_MAP_EMPLACE(base_properties, "assert", VIA_LIB_WRAP_CFPTR(base_assert));

    for (const auto& [ident, val] : base_properties) {
        __set_global(V, ident, val);
    }
}

VIA_NAMESPACE_END
