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
        TValue *argx = getargument(V, 0);
        oss << tostring(V, *argx).val_string->ptr << " ";
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
        TValue *argx = getargument(V, 0);
        oss << tostring(V, *argx).val_string->ptr << " ";
    }

    // Output the accumulated string
    std::cout << oss.str() << "\n";
}

void base_error(RTState *V)
{
    TValue *arg0 = getargument(V, 0);
    tostring(V, *arg0);
    V->abrt = true;
    LIB_ASSERT(false, arg0->val_string->ptr);
}

void base_exit(RTState *V)
{
    TValue *arg0 = getargument(V, 0);

    LIB_ASSERT(checknumber(V, *arg0), "Expected type TNumber for argument 0 of base_exit");

    ExitCode code = arg0->val_number;
    setexitdata(V, code, "base_exit called by user");
    V->abrt = true; // Abort the VM execution
}

void base_type(RTState *V)
{
    TValue *arg0 = getargument(V, 0);
    TValue ty = type(V, *arg0);
    pushret(V, ty);
}

void base_typeof(RTState *V)
{
    TValue *arg0 = getargument(V, 0);
    TValue type = typeofv(V, *arg0);
    pushret(V, type);
}

void base_tostring(RTState *V)
{
    TValue *arg0 = getargument(V, 0);
    TValue str = tostring(V, *arg0);
    pushret(V, str);
}

void base_tonumber(RTState *V)
{
    TValue *arg0 = getargument(V, 0);
    TValue num = tonumber(V, *arg0);
    pushret(V, num);
}

void base_tobool(RTState *V)
{
    TValue *arg0 = getargument(V, 0);
    TValue boole = tobool(V, *arg0);
    pushret(V, boole);
}

void base_assert(RTState *V)
{
    TValue *arg0 = getargument(V, 0);
    TValue *arg1 = getargument(V, 1);

    if (!tobool(V, *arg0).val_boolean)
    {
        TString *mvstr = tostring(V, *arg1).val_string;
        std::string mfstr = std::format("base_assert assertion failed: {}", mvstr->ptr);
        TString *mfstrds = newstring(V, mfstr.c_str());

        // Push the error message onto the argument stack
        pusharg(V, stackvalue(V, mfstrds));
        // Hack solution, but works!
        call(V, WRAPVAL(base_error));
    }
}

void base_getmetatable(RTState *V)
{
    TValue *arg0 = getargument(V, 0);

    LIB_ASSERT(checktable(V, *arg0), "base_getmetatable expects a table");

    TValue meta = getmetatable(V, arg0->val_table);
    pushret(V, meta);
}

void base_setmetatable(RTState *V)
{
    TValue *tbl = getargument(V, 0);
    TValue *meta = getargument(V, 0);

    LIB_ASSERT(checktable(V, *tbl), "base_setmetatable expects a table for argument 0");
    LIB_ASSERT(checktable(V, *meta), "base_setmetatable expects a table for argument 1");

    setmetatable(V, tbl->val_table, meta->val_table);
}

void base_pcall(RTState *V)
{
    TValue *callback = getargument(V, 0);
    // Realigns arguments by popping them and pushing them again
    for (size_t i = 0; i < V->argc; i++)
    {
        TValue *arg = getargument(V, 0);
        pusharg(V, *arg);
    }

    call(V, *callback);

    TBool success = V->exitc != 1;
    if (success)
    {
        pusharg(V, stackvalue(V, success));
        pusharg(V, *popval(V));
    }
    else
    {
        const char *exit_message = dupstring(V->exitm);
        TString *exit_string = newstring(V, exit_message);
        TValue exit_value = stackvalue(V, exit_string);

        pusharg(V, stackvalue(V, success));
        pusharg(V, exit_value);
    }
}

void base_xpcall(RTState *V)
{
    TValue *callback = getargument(V, 0);
    TValue *handler = getargument(V, 0);

    // Realigns arguments by popping them and pushing them again
    for (size_t i = 0; i < V->argc; i++)
    {
        TValue *arg = getargument(V, 0);
        pusharg(V, *arg);
    }

    call(V, *callback);

    TBool success = V->exitc != 1;
    if (success)
    {
        pusharg(V, stackvalue(V, success));
        pusharg(V, *popval(V));
    }
    else
    {
        const char *exit_message = dupstring(V->exitm);
        TString *exit_string = newstring(V, exit_message);
        TValue exit_value = stackvalue(V, exit_string);

        pusharg(V, stackvalue(V, success));
        pusharg(V, exit_value);
    }

    call(V, *handler);
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
        // VarId id = hashstring(V, it.first);
    }
}

} // namespace via::lib
