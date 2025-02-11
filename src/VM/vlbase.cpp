/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vlbase.h"
#include "api.h"
#include "libutils.h"
#include "types.h"

namespace via::lib
{

void base_print(State *V)
{
    uint16_t i = 0;
    std::ostringstream oss;

    // Loop over argument count
    while (i++ < V->argc)
    {
        const TValue &argx = getargument(V, 0);
        oss << tostring(V, argx).val_string->ptr << " ";
    }

    // Output the accumulated string
    std::cout << oss.str();

    nativeret(V, 0);
}

// Identical to `base_print` but ends the string with a line break
void base_println(State *V)
{
    uint8_t i = 0;
    std::ostringstream oss;

    // Loop over argument count
    while (i++ < V->argc)
    {
        const TValue &argx = getargument(V, 0);
        oss << tostring(V, argx).val_string->ptr << " ";
    }

    // Output the accumulated string
    std::cout << oss.str() << "\n";

    nativeret(V, 0);
}

void base_error(State *V)
{
    const TValue &arg0 = getargument(V, 0);

    tostring(V, arg0);
    abort(V);
    ferror(arg0.val_string->ptr);
    setexitcode(V, VMEC::user_error);
    nativeret(V, 0);
}

void base_exit(State *V)
{
    const TValue &arg0 = getargument(V, 0);

    LIB_ASSERT(checknumber(V, arg0), VMEC::unexpected_argument, "Expected type number for argument 0 of base_exit");

    int code = arg0.val_number;

    setexitcode(V, static_cast<VMEC>(code));
    abort(V);
    nativeret(V, 0);
}

void base_type(State *V)
{
    const TValue &arg0 = getargument(V, 0);
    TValue ty = type(V, arg0);

    push(V, ty);
    nativeret(V, 1);
}

void base_typeof(State *V)
{
    const TValue &arg0 = getargument(V, 0);
    TValue type = typeofv(V, arg0);

    push(V, type);
    nativeret(V, 1);
}

void base_tostring(State *V)
{
    const TValue &arg0 = getargument(V, 0);
    TValue str = tostring(V, arg0);

    push(V, str);
    nativeret(V, 1);
}

void base_tonumber(State *V)
{
    const TValue &arg0 = getargument(V, 0);
    TValue num = tonumber(V, arg0);

    push(V, num);
    nativeret(V, 1);
}

void base_tobool(State *V)
{
    const TValue &arg0 = getargument(V, 0);
    TValue boole = tobool(V, arg0);

    push(V, boole);
    nativeret(V, 1);
}

void base_assert(State *V)
{
    const TValue &arg0 = getargument(V, 0);
    const TValue &arg1 = getargument(V, 1);

    if (!tobool(V, arg0).val_boolean)
    {
        TString *mvstr = tostring(V, arg1).val_string;
        std::string mfstr = std::format("base_assert assertion failed: {}", mvstr->ptr);
        TString *mfstrds = new TString(V, mfstr.c_str());
        TValue err_val(mfstrds);
        TValue err_fn = WRAPVAL(base_error);

        // Push the error message onto the argument stack
        push(V, err_val);
        // Hack solution, but works!
        call(V, err_fn, 1);
    }
}

void base_getmetatable(State *V)
{
    const TValue &arg0 = getargument(V, 0);

    LIB_ASSERT(checktable(V, arg0), VMEC::unexpected_argument, "base_getmetatable expects a table");

    TValue meta = getmetatable(V, arg0.val_table);
    push(V, meta);
    nativeret(V, 1);
}

void base_setmetatable(State *V)
{
    const TValue &tbl = getargument(V, 0);
    const TValue &meta = getargument(V, 0);

    LIB_ASSERT(checktable(V, tbl), VMEC::unexpected_argument, "base_setmetatable expects a table for argument 0");
    LIB_ASSERT(checktable(V, meta), VMEC::unexpected_argument, "base_setmetatable expects a table for argument 1");

    setmetatable(V, tbl.val_table, meta.val_table);
    nativeret(V, 0);
}

void loadbaselib(State *V)
{
    std::unordered_map<kGlobId, TValue> base_properties;

    // Emplace each entry instead of using an initializer list
    base_properties.emplace("print", WRAPVAL(base_print));
    base_properties.emplace("println", WRAPVAL(base_println));
    base_properties.emplace("error", WRAPVAL(base_error));
    base_properties.emplace("exit", WRAPVAL(base_exit));
    base_properties.emplace("type", WRAPVAL(base_type));
    base_properties.emplace("typeof", WRAPVAL(base_typeof));
    base_properties.emplace("tostring", WRAPVAL(base_tostring));
    base_properties.emplace("tonumber", WRAPVAL(base_tonumber));
    base_properties.emplace("tobool", WRAPVAL(base_tobool));
    base_properties.emplace("assert", WRAPVAL(base_assert));

    // Iterate and set the global variables
    for (const auto &[ident, val] : base_properties)
        setglobal(V, ident, val);
}

} // namespace via::lib
