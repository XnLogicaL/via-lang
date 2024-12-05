/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vlvec3.h"

namespace via::lib
{

// Forward declaration of the default vec3 constructor
void vec3_new(viaState *);

// Utility: Get a vector component (x, y, z) from a table
viaValue get_vec3_component(viaState *V, viaTable *vec, const char *key)
{
    return *via_gettableindex(V, vec, viaT_hashstring(V, key), false);
}

// Utility: Perform a binary operation on two vectors
template<typename Op>
void vec3_binary_op(viaState *V, Op op)
{
    viaValue self = via_popargument(V);
    viaValue other = via_popargument(V);

    const char *keys[] = {"x", "y", "z"};
    viaValue results[3];

    for (int i = 0; i < 3; ++i)
    {
        viaNumber a = get_vec3_component(V, self.val_table, keys[i]).val_number;
        viaNumber b = get_vec3_component(V, other.val_table, keys[i]).val_number;
        results[i] = viaT_stackvalue(V, op(a, b));
    }

    viaL_pusharguments(V, {results[0], results[1], results[2]});
    vec3_new(V);
}

// Utility: Perform a unary operation on a vector
template<typename Op>
void vec3_unary_op(viaState *V, Op op)
{
    viaValue self = via_popargument(V);

    viaRawString_t keys[] = {"x", "y", "z"};
    viaValue results[3];

    for (int i = 0; i < 3; ++i)
    {
        viaNumber a = get_vec3_component(V, self.val_table, keys[i]).val_number;
        results[i] = viaT_stackvalue(V, op(a));
    }

    viaL_pusharguments(V, {results[0], results[1], results[2]});
    vec3_new(V);
}

// Vector operations
void vec3_mmadd(viaState *V)
{
    vec3_binary_op(
        V,
        [](viaNumber a, viaNumber b)
        {
            return a + b;
        }
    );
}

void vec3_mmsub(viaState *V)
{
    vec3_binary_op(
        V,
        [](viaNumber a, viaNumber b)
        {
            return a - b;
        }
    );
}

void vec3_mmmul(viaState *V)
{
    vec3_binary_op(
        V,
        [](viaNumber a, viaNumber b)
        {
            return a * b;
        }
    );
}

void vec3_mmdiv(viaState *V)
{
    vec3_binary_op(
        V,
        [](viaNumber a, viaNumber b)
        {
            return a / b;
        }
    );
}

void vec3_mmunm(viaState *V)
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
void vec3_magnitude(viaState *V)
{
    viaTable *self = via_popargument(V).val_table;

    viaNumber x = get_vec3_component(V, self, "x").val_number;
    viaNumber y = get_vec3_component(V, self, "y").val_number;
    viaNumber z = get_vec3_component(V, self, "z").val_number;

    viaValue mag = viaT_stackvalue(V, std::sqrt(x * x + y * y + z * z));
    via_pushreturn(V, mag);
}

// Normalize function
void vec3_normalize(viaState *V)
{
    viaTable *self = via_popargument(V).val_table;

    vec3_magnitude(V); // Compute magnitude
    viaNumber mag = via_popreturn(V).val_number;

    const char *keys[] = {"x", "y", "z"};
    viaValue results[3];

    for (int i = 0; i < 3; ++i)
    {
        viaNumber a = get_vec3_component(V, self, keys[i]).val_number;
        results[i] = viaT_stackvalue(V, a / mag);
    }

    viaL_pusharguments(V, {results[0], results[1], results[2]});
    vec3_new(V);
}

// Create vector
void vec3_new(viaState *V)
{
    viaTable *vec3_ins = viaT_newtable(V, vec3_meta);
    viaRawString_t keys[] = {"x", "y", "z"};

    for (int i = 0; i < 3; ++i)
    {
        viaValue val = via_popargument(V);
        via_settableindex(V, vec3_ins, viaT_hashstring(V, keys[i]), val);
    }

    via_pushreturn(V, viaT_stackvalue(V, vec3_ins));
}

// Predefined vectors
void vec3_one(viaState *V)
{
    viaL_pusharguments(V, {viaT_stackvalue(V, 1.0f), viaT_stackvalue(V, 1.0f), viaT_stackvalue(V, 1.0f)});
    vec3_new(V);
}

void vec3_zero(viaState *V)
{
    viaL_pusharguments(V, {viaT_stackvalue(V, 0.0f), viaT_stackvalue(V, 0.0f), viaT_stackvalue(V, 0.0f)});
    vec3_new(V);
}

// Library loader
void viaL_loadvec3lib(viaState *V)
{
    static const viaHashMap_t<viaRawString_t, viaValue> vec3_meta_properties = {
        {"magnitude", viaT_stackvalue(V, vec3_magnitude)},
        {"normalize", viaT_stackvalue(V, vec3_normalize)},
        {"__add", viaT_stackvalue(V, vec3_mmadd)},
        {"__sub", viaT_stackvalue(V, vec3_mmsub)},
        {"__mul", viaT_stackvalue(V, vec3_mmmul)},
        {"__div", viaT_stackvalue(V, vec3_mmdiv)},
        {"__unm", viaT_stackvalue(V, vec3_mmunm)}
    };

    static const viaHashMap_t<viaRawString_t, viaValue> vec3_gtable_properties = {
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
    via_loadlib(V, viaT_hashstring(V, "vector3"), viaT_stackvalue(V, lib));
}

} // namespace via::lib