/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "api.h"
#include "bytecode.h"
#include "libutils.h"
#include "types.h"
#include "state.h"
#include "instruction.h"

namespace via::lib
{

using namespace VM;

inline void std_print(viaState *V)
{
    uint8_t i = 0;
    std::ostringstream oss;

    // Loop over the first 16 registers
    while (i < 16)
    {
        viaRegister R = __via_get_arg_register(i++);
        viaValue RV = *via_getregister(V, R);

        if (RV.type == viaValueType::Nil)
            break; // Stop if Nil is encountered

        oss << via_tostring(V, RV).str << " ";
    }

    std::cout << oss.str() << "\n"; // Output the accumulated string

    return;
}

inline void std_error(viaState *V)
{
    viaRegister R = __via_get_arg_register(0);
    viaValue RV = *via_getregister(V, R);

    via_fatalerr(V, via_tostring(V, RV).str);

    return;
}

inline void std_exit(viaState *V)
{
    viaRegister R = __via_get_arg_register(0);
    viaValue EC = *via_getregister(V, R);

    LIB_ASSERT(EC.type == viaValueType::viaNumber, "Expected type viaNumber for argument 0 of std_exit");

    int code = static_cast<int>(EC.num);

    via_setexitdata(V, code, "std_exit called by user");
    V->abrt = true; // Abort the VM execution

    return;
}

inline void std_type(viaState *V)
{
    viaRegister R = __via_get_arg_register(0);
    viaValue RV = *via_getregister(V, R);
    viaRegister RR = __via_get_ret_register(0);

    via_setregister(V, RR, via_type(V, RV));

    return;
}

inline void std_typeof(viaState *V)
{
    viaRegister R = __via_get_arg_register(0);
    viaValue RV = *via_getregister(V, R);
    viaRegister RR = __via_get_ret_register(0);

    via_setregister(V, RR, via_typeof(V, RV));

    return;
}

inline void std_tostring(viaState *V)
{
    viaRegister R = __via_get_arg_register(0);
    viaValue RV = *via_getregister(V, R);
    viaRegister RR = __via_get_ret_register(0);

    via_setregister(V, RR, via_tostring(V, RV));

    return;
}

inline void std_tonumber(viaState *V)
{
    viaRegister R = __via_get_arg_register(0);
    viaValue RV = *via_getregister(V, R);
    viaRegister RR = __via_get_ret_register(0);

    via_setregister(V, RR, via_tonumber(V, RV));

    return;
}

inline void std_tobool(viaState *V)
{
    viaRegister R = __via_get_arg_register(0);
    viaValue RV = *via_getregister(V, R);
    viaRegister RR = __via_get_ret_register(0);

    via_setregister(V, RR, via_tobool(V, RV));

    return;
}

inline void std_assert(viaState *V)
{
    viaRegister cr = __via_get_arg_register(0);
    viaRegister mr = __via_get_arg_register(1);

    viaValue cv = *via_getregister(V, cr);
    viaValue mv = *via_getregister(V, mr);

    if (!via_tobool(V, cv).boole)
    {
        String mvstr = via_tostring(V, mv).str;
        std::string mfstr = std::format("std_assert assertion failed: {}", mvstr);
        String mfstrds = strdup(mfstr.c_str());

        via_setregister(V, cr, mfstrds); // Set the error message in the first register

        std_error(V); // Invoke the error handler
    }

    return;
}

inline void vstl_load(viaState *V)
{
    via_setglobal(V, "print", viaValue(std_print));
    via_setglobal(V, "error", viaValue(std_error));
    via_setglobal(V, "exit", viaValue(std_exit));
    via_setglobal(V, "type", viaValue(std_type));
    via_setglobal(V, "typeof", viaValue(std_typeof));
    via_setglobal(V, "tostring", viaValue(std_tostring));
    via_setglobal(V, "tonumber", viaValue(std_tonumber));
    via_setglobal(V, "tobool", viaValue(std_tobool));
    via_setglobal(V, "assert", viaValue(std_assert));
}

} // namespace via::lib
