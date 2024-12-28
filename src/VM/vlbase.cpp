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
        TValue argx = popargument(V);
        oss << tostring(V, argx).val_string->ptr << " ";
    }

    // Output the accumulated string
    std::cout << oss.str();
}

// Identical to `base_print` but ends the string with a line break
void base_println(RTState *V)
{
    uint8_t i = 0;
    std::ostringstream oss;

    // Loop over argument count
    while (i++ < V->argc)
    {
        TValue argx = popargument(V);
        oss << tostring(V, argx).val_string->ptr << " ";
    }

    // Output the accumulated string
    std::cout << oss.str() << "\n";
}

void base_error(RTState *V)
{
    TValue arg0 = popargument(V);
    tostring(V, arg0);
    V->abrt = true;
    LIB_ASSERT(false, arg0.val_string->ptr);
}

void base_exit(RTState *V)
{
    TValue arg0 = popargument(V);

    LIB_ASSERT(arg0.type == ValueType::Number, "Expected type TNumber for argument 0 of base_exit");

    ExitCode code = arg0.val_number;
    setexitdata(V, code, "base_exit called by user");
    V->abrt = true; // Abort the VM execution
}

void base_type(RTState *V)
{
    TValue arg0 = popargument(V);
    TValue ty = type(V, arg0);
    pushreturn(V, ty);
}

void base_typeof(RTState *V)
{
    TValue arg0 = popargument(V);
    TValue type = typeof(V, arg0);
    pushreturn(V, type);
}

void base_tostring(RTState *V)
{
    TValue arg0 = popargument(V);
    TValue str = tostring(V, arg0);
    pushreturn(V, str);
}

void base_tonumber(RTState *V)
{
    TValue arg0 = popargument(V);
    TValue num = tonumber(V, arg0);
    pushreturn(V, num);
}

void base_tobool(RTState *V)
{
    TValue arg0 = popargument(V);
    TValue boole = tobool(V, arg0);
    pushreturn(V, boole);
}

void base_assert(RTState *V)
{
    TValue arg0 = popargument(V);
    TValue arg1 = popargument(V);

    if (!tobool(V, arg0).val_boolean)
    {
        TString *mvstr = tostring(V, arg1).val_string;
        std::string mfstr = std::format("base_assert assertion failed: {}", mvstr->ptr);
        TString *mfstrds = newstring(V, mfstr.c_str());

        // Push the error message onto the argument stack
        pushargument(V, stackvalue(V, mfstrds));

        // Hack solution, but works!
        call(V, WRAPVAL(base_error));
    }
}

void base_getmetatable(RTState *V)
{
    TValue arg0 = popargument(V);

    LIB_ASSERT(checktable(V, arg0), "base_getmetatable expects a table");

    TValue meta = getmetatable(V, arg0.val_table);
    pushreturn(V, meta);
}

void base_setmetatable(RTState *V)
{
    TValue tbl = popargument(V);
    TValue meta = popargument(V);

    LIB_ASSERT(checktable(V, tbl), "base_setmetatable expects a table for argument 0");
    LIB_ASSERT(checktable(V, meta), "base_setmetatable expects a table for argument 1");

    setmetatable(V, tbl.val_table, meta.val_table);
}

void base_pcall(RTState *V)
{
    TValue callback = popargument(V);
    // Realigns arguments by popping them and pushing them again
    for (size_t i = 0; i < V->argc; i++)
    {
        TValue arg = popargument(V);
        pushargument(V, arg);
    }

    call(V, callback);

    TBool success = V->exitc != 1;
    if (success)
    {
        pushargument(V, stackvalue(V, success));
        pushargument(V, popreturn(V));
    }
    else
    {
        const char *exit_message = dupstring(V->exitm);
        TString *exit_string = newstring(V, exit_message);
        TValue exit_value = stackvalue(V, exit_string);

        pushargument(V, stackvalue(V, success));
        pushargument(V, exit_value);
    }
}

void base_xpcall(RTState *V)
{
    TValue callback = popargument(V);
    TValue handler = popargument(V);

    // Realigns arguments by popping them and pushing them again
    for (size_t i = 0; i < V->argc; i++)
    {
        TValue arg = popargument(V);
        pushargument(V, arg);
    }

    call(V, callback);

    TBool success = V->exitc != 1;
    if (success)
    {
        pushargument(V, stackvalue(V, success));
        pushargument(V, popreturn(V));
    }
    else
    {
        const char *exit_message = dupstring(V->exitm);
        TString *exit_string = newstring(V, exit_message);
        TValue exit_value = stackvalue(V, exit_string);

        pushargument(V, stackvalue(V, success));
        pushargument(V, exit_value);
    }

    call(V, handler);
}

void loadbaselib(RTState *V)
{
    HashMap<const char *, TValue> base_properties = {
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
        {"pcall", stackvalue(V, newcfunc(V, base_pcall, true))},
        {"xpcall", stackvalue(V, newcfunc(V, base_xpcall, true))},
    };

    for (auto it : base_properties)
    {
        VarId id = hashstring(V, it.first);
        setglobal(V, id, it.second);
    }
}

} // namespace via::lib
