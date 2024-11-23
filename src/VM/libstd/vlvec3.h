/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "bytecode.h"
#include "shared.h"
#include "state.h"
#include "api.h"
#include "types.h"
#include "libutils.h"

#include <cmath>

namespace via::lib
{

static viaTable *vec3_meta = new viaTable;

// Forward declaration of the default vec3 constructor
inline void vec3_new(viaState *);

// Utility: Get a vector component (x, y, z) from a table
inline viaValue get_vec3_component(viaState *V, viaTable *vec, const char *key)
{
    return *via_gettableindex(V, vec, viaT_hashstring(V, key), false);
}

// Utility: Perform a binary operation on two vectors
template<typename Op>
inline void vec3_binary_op(viaState *V, Op op)
{
    viaValue *self = via_getregister(V, viaL_getargument(V, 0));
    viaValue *other = via_getregister(V, viaL_getargument(V, 1));

    const char *keys[] = {"x", "y", "z"};
    viaValue results[3];

    for (int i = 0; i < 3; ++i)
    {
        viaNumber a = get_vec3_component(V, self->tbl, keys[i]).num;
        viaNumber b = get_vec3_component(V, other->tbl, keys[i]).num;
        results[i] = viaT_stackvalue(V, op(a, b));
    }

    viaL_loadargs(V, {results[0], results[1], results[2]});
    vec3_new(V);
}

// Utility: Perform a unary operation on a vector
template<typename Op>
inline void vec3_unary_op(viaState *V, Op op)
{
    viaValue *self = via_getregister(V, viaL_getargument(V, 0));

    const char *keys[] = {"x", "y", "z"};
    viaValue results[3];

    for (int i = 0; i < 3; ++i)
    {
        viaNumber a = get_vec3_component(V, self->tbl, keys[i]).num;
        results[i] = viaT_stackvalue(V, op(a));
    }

    viaL_loadargs(V, {results[0], results[1], results[2]});
    vec3_new(V);
}

// Vector operations
inline void vec3_mmadd(viaState *V)
{
    vec3_binary_op(
        V,
        [](viaNumber a, viaNumber b)
        {
            return a + b;
        }
    );
}

inline void vec3_mmsub(viaState *V)
{
    vec3_binary_op(
        V,
        [](viaNumber a, viaNumber b)
        {
            return a - b;
        }
    );
}

inline void vec3_mmmul(viaState *V)
{
    vec3_binary_op(
        V,
        [](viaNumber a, viaNumber b)
        {
            return a * b;
        }
    );
}

inline void vec3_mmdiv(viaState *V)
{
    vec3_binary_op(
        V,
        [](viaNumber a, viaNumber b)
        {
            return a / b;
        }
    );
}

inline void vec3_mmunm(viaState *V)
{
    vec3_unary_op(
        V,
        [](viaNumber a)
        {
            return -a;
        }
    );
}

// Magnitude function
inline void vec3_magnitude(viaState *V)
{
    viaRegister selfR = viaL_getargument(V, 0);
    viaTable *self = via_getregister(V, selfR)->tbl;

    viaNumber x = get_vec3_component(V, self, "x").num;
    viaNumber y = get_vec3_component(V, self, "y").num;
    viaNumber z = get_vec3_component(V, self, "z").num;

    viaValue mag = viaT_stackvalue(V, std::sqrt(x * x + y * y + z * z));
    via_setregister(V, viaL_getreturn(V, 0), mag);
}

// Normalize function
inline void vec3_normalize(viaState *V)
{
    viaRegister selfR = viaL_getargument(V, 0);
    viaTable *self = via_getregister(V, selfR)->tbl;

    vec3_magnitude(V); // Compute magnitude
    viaNumber mag = via_getregister(V, viaL_getreturn(V, 0))->num;

    const char *keys[] = {"x", "y", "z"};
    viaValue results[3];

    for (int i = 0; i < 3; ++i)
    {
        viaNumber a = get_vec3_component(V, self, keys[i]).num;
        results[i] = viaT_stackvalue(V, a / mag);
    }

    viaL_loadargs(V, {results[0], results[1], results[2]});
    vec3_new(V);

    via_setregister(V, viaL_getreturn(V, 0), *via_getregister(V, viaL_getreturn(V, 0)));
}

// Create vector
inline void vec3_new(viaState *V)
{
    viaTable *vec3_ins = viaT_newtable(V, vec3_meta);

    const char *keys[] = {"x", "y", "z"};
    for (int i = 0; i < 3; ++i)
    {
        viaValue *val = via_getregister(V, viaL_getargument(V, i));
        via_settableindex(V, vec3_ins, viaT_hashstring(V, keys[i]), *val);
    }

    via_setregister(V, viaL_getreturn(V, 0), viaT_stackvalue(V, vec3_ins));
}

// Predefined vectors
inline void vec3_one(viaState *V)
{
    viaL_loadargs(V, {viaT_stackvalue(V, 1.0f), viaT_stackvalue(V, 1.0f), viaT_stackvalue(V, 1.0f)});
    vec3_new(V);
}

inline void vec3_zero(viaState *V)
{
    viaL_loadargs(V, {viaT_stackvalue(V, 0.0f), viaT_stackvalue(V, 0.0f), viaT_stackvalue(V, 0.0f)});
    vec3_new(V);
}

// Library loader
inline void viaL_loadvec3lib(viaState *V)
{
    static const viaHashMap<viaRawString, viaValue> vec3_meta_properties = {
        {"magnitude", viaT_stackvalue(V, vec3_magnitude)},
        {"normalize", viaT_stackvalue(V, vec3_normalize)},
        {"__add", viaT_stackvalue(V, vec3_mmadd)},
        {"__sub", viaT_stackvalue(V, vec3_mmsub)},
        {"__mul", viaT_stackvalue(V, vec3_mmmul)},
        {"__div", viaT_stackvalue(V, vec3_mmdiv)},
        {"__unm", viaT_stackvalue(V, vec3_mmunm)}
    };

    static const viaHashMap<viaRawString, viaValue> vec3_gtable_properties = {
        {"new", viaT_stackvalue(V, vec3_new)},
        {"one", viaT_stackvalue(V, vec3_one)},
        {"zero", viaT_stackvalue(V, vec3_zero)},
    };

    viaTable *lib = viaT_newtable(V);

    for (auto it : vec3_meta_properties)
        via_settableindex(V, vec3_meta, viaT_hashstring(V, it.first), it.second);

    for (auto it : vec3_gtable_properties)
        via_settableindex(V, lib, viaT_hashstring(V, it.first), it.second);

    via_freeze(V, lib);
    via_loadlib(V, "vector3", viaT_stackvalue(V, lib));
}

} // namespace via::lib
