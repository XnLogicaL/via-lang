/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vlbase.h"
#include "api.h"
#include "libutils.h"
#include "types.h"

namespace via::lib
{

void base_print(RTState *V)
{
    uint16_t i = 0;
    std::ostringstream oss;

    // Loop over argument count
    while (i++ < V->argc)
    {
        TValue argx = getargument(V, 0);
        oss << tostring(V, argx).val_string->ptr << " ";
    }

    // Output the accumulated string
    std::cout << oss.str();

    nativeret(V, 0);
}

// Identical to `base_print` but ends the string with a line break
void base_println(RTState *V)
{
    uint8_t i = 0;
    std::ostringstream oss;

    // Loop over argument count
    while (i++ < V->argc)
    {
        TValue argx = getargument(V, 0);
        oss << tostring(V, argx).val_string->ptr << " ";
    }

    // Output the accumulated string
    std::cout << oss.str() << "\n";

    nativeret(V, 0);
}

void base_error(RTState *V)
{
    TValue arg0 = getargument(V, 0);

    tostring(V, arg0);

    V->abrt = true;

    LIB_ASSERT(false, arg0.val_string->ptr);
    nativeret(V, 0);
}

void base_exit(RTState *V)
{
    TValue arg0 = getargument(V, 0);

    LIB_ASSERT(checknumber(V, arg0), "Expected type TNumber for argument 0 of base_exit");

    ExitCode code = arg0.val_number;
    setexitdata(V, code, "base_exit called by user");
    V->abrt = true; // Abort the VM execution

    nativeret(V, 0);
}

void base_type(RTState *V)
{
    TValue arg0 = getargument(V, 0);
    TValue ty = type(V, arg0);

    pushval(V, ty);
    nativeret(V, 1);
}

void base_typeof(RTState *V)
{
    TValue arg0 = getargument(V, 0);
    TValue type = typeofv(V, arg0);

    pushval(V, type);
    nativeret(V, 1);
}

void base_tostring(RTState *V)
{
    TValue arg0 = getargument(V, 0);
    TValue str = tostring(V, arg0);

    pushval(V, str);
    nativeret(V, 1);
}

void base_tonumber(RTState *V)
{
    TValue arg0 = getargument(V, 0);
    TValue num = tonumber(V, arg0);

    pushval(V, num);
    nativeret(V, 1);
}

void base_tobool(RTState *V)
{
    TValue arg0 = getargument(V, 0);
    TValue boole = tobool(V, arg0);

    pushval(V, boole);
    nativeret(V, 1);
}

void base_assert(RTState *V)
{
    TValue arg0 = getargument(V, 0);
    TValue arg1 = getargument(V, 1);

    if (!tobool(V, arg0).val_boolean)
    {
        TString *mvstr = tostring(V, arg1).val_string;
        std::string mfstr = std::format("base_assert assertion failed: {}", mvstr->ptr);
        TString *mfstrds = new TString(V, mfstr.c_str());

        // Push the error message onto the argument stack
        pushval(V, TValue(mfstrds));
        // Hack solution, but works!
        call(V, WRAPVAL(base_error), 1);
    }
}

void base_getmetatable(RTState *V)
{
    TValue arg0 = getargument(V, 0);

    LIB_ASSERT(checktable(V, arg0), "base_getmetatable expects a table");

    TValue meta = getmetatable(V, arg0.val_table);
    pushval(V, meta);
    nativeret(V, 1);
}

void base_setmetatable(RTState *V)
{
    TValue tbl = getargument(V, 0);
    TValue meta = getargument(V, 0);

    LIB_ASSERT(checktable(V, tbl), "base_setmetatable expects a table for argument 0");
    LIB_ASSERT(checktable(V, meta), "base_setmetatable expects a table for argument 1");

    setmetatable(V, tbl.val_table, meta.val_table);
    nativeret(V, 0);
}

void base_pcall(RTState *V)
{
    TValue callback = getargument(V, 0);
    // Realigns arguments by popping them and pushing them again
    for (size_t i = 0; i < V->argc; i++)
    {
        TValue arg = getargument(V, i);
        pushval(V, arg);
    }

    call(V, callback, V->argc - 1);

    TBool success = V->exitc != 1;
    if (success)
    {
        pushval(V, TValue(success));
        pushval(V, popval(V));
    }
    else
    {
        const char *exit_message = dupstring(V->exitm);
        TString *exit_string = new TString(V, exit_message);
        TValue exit_value = TValue(exit_string);

        pushval(V, TValue(success));
        pushval(V, exit_value);
    }

    nativeret(V, 2);
}

void base_xpcall(RTState *V)
{
    TValue callback = getargument(V, 0);
    TValue handler = getargument(V, 1);

    // Realigns arguments by popping them and pushing them again
    for (size_t i = 0; i < V->argc; i++)
    {
        TValue arg = getargument(V, i);
        pushval(V, arg);
    }

    call(V, callback, V->argc - 2);

    TBool success = V->exitc != 1;
    if (success)
    {
        pushval(V, TValue(success));
        pushval(V, popval(V));
    }
    else
    {
        const char *exit_message = dupstring(V->exitm);
        TString *exit_string = new TString(V, exit_message);
        TValue exit_value = TValue(exit_string);

        pushval(V, exit_value);
        call(V, handler, 1);
        pushval(V, TValue(success));
        pushval(V, exit_value);
    }

    nativeret(V, 2);
}

void loadbaselib(RTState *V)
{
    HashMap<kGlobId, TValue> base_properties = {
        {"print", WRAPVAL(base_print)},
        {"println", WRAPVAL(base_println)},
        {"error", WRAPVAL(base_error)},
        {"exit", WRAPVAL(base_exit)},
        {"type", WRAPVAL(base_type)},
        {"typeof", WRAPVAL(base_typeof)},
        {"tostring", WRAPVAL(base_tostring)},
        {"tonumber", WRAPVAL(base_tonumber)},
        {"tobool", WRAPVAL(base_tobool)},
        {"assert", WRAPVAL(base_assert)},
        {"pcall", TValue(new TCFunction(base_pcall, true))},
        {"xpcall", TValue(new TCFunction(base_xpcall, true))},
    };

    for (const auto [ident, val] : base_properties)
        setglobal(V, ident, val);
}

} // namespace via::lib
