/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vlbase.h"
#include "api.h"
#include "libutils.h"
#include "types.h"

namespace via::lib
{

void base_print(viaState *V)
{
    uint16_t i = 0;
    std::ostringstream oss;

    // Loop over argument count
    while (i < V->argc)
    {
        viaValue argx = via_popargument(V);
        oss << via_tostring(V, argx).val_string->ptr << " ";
        i++;
    }

    // Output the accumulated string
    std::cout << oss.str();
}

// Basically `base_print` but ends the string with a line break
void base_println(viaState *V)
{
    uint8_t i = 0;
    std::ostringstream oss;

    // Loop over argument count
    while (i < V->argc)
    {
        viaValue argx = via_popargument(V);
        oss << via_tostring(V, argx).val_string->ptr << " ";
        i++;
    }

    // Output the accumulated string
    std::cout << oss.str() << "\n";
}

void base_error(viaState *V)
{
    viaValue arg0 = via_popargument(V);
    via_fatalerr(V, via_tostring(V, arg0).val_string->ptr);
}

void base_exit(viaState *V)
{
    viaValue arg0 = via_popargument(V);

    LIB_ASSERT(arg0.type == viaValueType::Number, "Expected type viaNumber for argument 0 of base_exit");

    viaExitCode_t code = arg0.val_number;
    via_setexitdata(V, code, "base_exit called by user");
    V->abrt = true; // Abort the VM execution
}

void base_type(viaState *V)
{
    viaValue arg0 = via_popargument(V);
    viaValue type = via_type(V, arg0);
    via_pushreturn(V, type);
}

void base_typeof(viaState *V)
{
    viaValue arg0 = via_popargument(V);
    viaValue type = via_typeof(V, arg0);
    via_pushreturn(V, type);
}

void base_tostring(viaState *V)
{
    viaValue arg0 = via_popargument(V);
    viaValue str = via_tostring(V, arg0);
    via_pushreturn(V, str);
}

void base_tonumber(viaState *V)
{
    viaValue arg0 = via_popargument(V);
    viaValue num = via_tonumber(V, arg0);
    via_pushreturn(V, num);
}

void base_tobool(viaState *V)
{
    viaValue arg0 = via_popargument(V);
    viaValue boole = via_tobool(V, arg0);
    via_pushreturn(V, boole);
}

void base_assert(viaState *V)
{
    viaValue arg0 = via_popargument(V);
    viaValue arg1 = via_popargument(V);

    if (!via_tobool(V, arg0).val_boolean)
    {
        viaString *mvstr = via_tostring(V, arg1).val_string;
        std::string mfstr = std::format("base_assert assertion failed: {}", mvstr->ptr);
        viaString *mfstrds = viaT_newstring(V, mfstr.c_str());

        // Push the error message onto the argument stack
        via_pushargument(V, viaT_stackvalue(V, mfstrds));

        // Hack solution, but works!
        via_call(V, WRAPVAL(base_error));
    }
}

void base_getmetatable(viaState *V)
{
    viaValue arg0 = via_popargument(V);

    LIB_ASSERT(viaT_checktable(V, arg0), "base_getmetatable expects a table");

    viaValue meta = via_getmetatable(V, arg0.val_table);
    via_pushreturn(V, meta);
}

void base_setmetatable(viaState *V)
{
    viaValue tbl = via_popargument(V);
    viaValue meta = via_popargument(V);

    LIB_ASSERT(viaT_checktable(V, tbl), "base_setmetatable expects a table for argument 0");
    LIB_ASSERT(viaT_checktable(V, meta), "base_setmetatable expects a table for argument 1");

    via_setmetatable(V, tbl.val_table, meta.val_table);
}

void viaL_loadbaselib(viaState *V)
{
    viaHashMap_t<viaRawString_t, viaValue> base_properties = {
        {"print", WRAPVAL(base_print)},
        {"println", WRAPVAL(base_println)},
        {"error", WRAPVAL(base_error)},
        {"exit", WRAPVAL(base_exit)},
        {"type", WRAPVAL(base_type)},
        {"typeof", WRAPVAL(base_typeof)},
        {"tostring", WRAPVAL(base_tostring)},
        {"tonumber", WRAPVAL(base_tonumber)},
        {"tobool", WRAPVAL(base_tobool)},
        {"assert", WRAPVAL(base_assert)}
    };

    for (auto it : base_properties)
    {
        viaGlobalIdentifier_t id = it.first;
        via_setglobal(V, id, it.second);
    }
}

} // namespace via::lib
