/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vlbase.h"
#include "api.h"
#include "libutils.h"
#include "types.h"

namespace via::lib
{

LIB_DECL_FUNCTION(base_print)
{
    uint16_t i = 0;
    std::ostringstream oss;

    while (i++ < V->argc)
    {
        LIB_DECL_PARAMETER(argx, i);
        oss << tocxxstring(V, argx) << " ";
    }

    std::cout << oss.str();

    LIB_RETURN(0)
}

LIB_DECL_FUNCTION(base_println)
{
    uint8_t i = 0;
    std::ostringstream oss;

    while (i++ < V->argc)
    {
        LIB_DECL_PARAMETER(argx, i);
        oss << tocxxstring(V, argx) << " ";
    }

    std::cout << oss.str() << "\n";

    LIB_RETURN(0);
}

LIB_DECL_FUNCTION(base_error)
{
    LIB_DECL_PARAMETER(arg0, 0);

    tostring(V, arg0);
    abort(V);
    ferror(arg0.val_string->ptr);
    setexitcode(V, VMEC::user_error);

    LIB_RETURN(0);
}

LIB_DECL_FUNCTION(base_exit)
{
    LIB_DECL_PARAMETER(arg0, 0);

    if (!checknumber(arg0))
        LIB_ERR_ARG_TYPE_MISMATCH("number", ENUM_NAME(arg0.type), 0);

    int code = arg0.val_number;
    setexitcode(V, static_cast<VMEC>(code));
    abort(V);

    LIB_RETURN(0);
}

LIB_DECL_FUNCTION(base_type)
{
    LIB_DECL_PARAMETER(arg0, 0);

    TValue ty = type(V, arg0);
    push(V, ty);

    LIB_RETURN(1);
}

LIB_DECL_FUNCTION(base_typeof)
{
    LIB_DECL_PARAMETER(arg0, 0);

    TValue type = typeofv(V, arg0);
    push(V, type);

    LIB_RETURN(1);
}

LIB_DECL_FUNCTION(base_assert)
{
    LIB_DECL_PARAMETER(arg0, 0);
    LIB_DECL_PARAMETER(arg1, 1);

    if (!tocxxbool(arg0))
    {
        std::string err_cxx_str = std::format("base_assert assertion failed: {}", tocxxstring(V, arg1));
        TString *err_str = new TString(V, err_cxx_str.c_str());

        TValue err_val(err_str);
        TValue err_fn = LIB_WRAP_CFPTR(base_error);

        push(V, err_val);
        call(V, err_fn, 1);
    }

    LIB_RETURN(0);
}

LIB_DECL_FUNCTION(loadbaselib)
{
    std::unordered_map<kGlobId, TValue> base_properties;

    base_properties.emplace("print", LIB_WRAP_CFPTR(base_print));
    base_properties.emplace("println", LIB_WRAP_CFPTR(base_println));
    base_properties.emplace("error", LIB_WRAP_CFPTR(base_error));
    base_properties.emplace("exit", LIB_WRAP_CFPTR(base_exit));
    base_properties.emplace("type", LIB_WRAP_CFPTR(base_type));
    base_properties.emplace("typeof", LIB_WRAP_CFPTR(base_typeof));
    base_properties.emplace("assert", LIB_WRAP_CFPTR(base_assert));

    for (const auto &[ident, val] : base_properties)
        setglobal(V, ident, val);
}

} // namespace via::lib
