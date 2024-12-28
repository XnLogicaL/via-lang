/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vlvec3.h"

namespace via::lib
{

// Forward declaration of the default vec3 constructor
void vec3_new(RTState *);

// Utility: Get a vector component (x, y, z) from a table
TValue get_vec3_component(RTState *V, TTable *vec, const char *key)
{
    return *gettableindex(V, vec, hashstring(V, key), false);
}

// Utility: Perform a binary operation on two vectors
template<typename Op>
void vec3_binary_op(RTState *V, Op op)
{
    TValue self = popargument(V);
    TValue other = popargument(V);

    const char *keys[] = {"x", "y", "z"};
    TValue results[3];

    for (int i = 0; i < 3; ++i)
    {
        TNumber a = get_vec3_component(V, self.val_table, keys[i]).val_number;
        TNumber b = get_vec3_component(V, other.val_table, keys[i]).val_number;
        results[i] = stackvalue(V, op(a, b));
    }

    pusharguments(V, {results[0], results[1], results[2]});
    vec3_new(V);
}

// Utility: Perform a unary operation on a vector
template<typename Op>
void vec3_unary_op(RTState *V, Op op)
{
    TValue self = popargument(V);

    const char *keys[] = {"x", "y", "z"};
    TValue results[3];

    for (int i = 0; i < 3; ++i)
    {
        TNumber a = get_vec3_component(V, self.val_table, keys[i]).val_number;
        results[i] = stackvalue(V, op(a));
    }

    pusharguments(V, {results[0], results[1], results[2]});
    vec3_new(V);
}

// Vector operations
void vec3_mmadd(RTState *V)
{
    vec3_binary_op(
        V,
        [](TNumber a, TNumber b)
        {
            return a + b;
        }
    );
}

void vec3_mmsub(RTState *V)
{
    vec3_binary_op(
        V,
        [](TNumber a, TNumber b)
        {
            return a - b;
        }
    );
}

void vec3_mmmul(RTState *V)
{
    vec3_binary_op(
        V,
        [](TNumber a, TNumber b)
        {
            return a * b;
        }
    );
}

void vec3_mmdiv(RTState *V)
{
    vec3_binary_op(
        V,
        [](TNumber a, TNumber b)
        {
            return a / b;
        }
    );
}

void vec3_mmunm(RTState *V)
{
    vec3_unary_op(
        V,
        [](TNumber a)
        {
            return -a;
        }
    );
}

// Magnitude function
void vec3_magnitude(RTState *V)
{
    TTable *self = popargument(V).val_table;

    TNumber x = get_vec3_component(V, self, "x").val_number;
    TNumber y = get_vec3_component(V, self, "y").val_number;
    TNumber z = get_vec3_component(V, self, "z").val_number;

    TValue mag = stackvalue(V, std::sqrt(x * x + y * y + z * z));
    pushreturn(V, mag);
}

// Normalize function
void vec3_normalize(RTState *V)
{
    TTable *self = popargument(V).val_table;

    vec3_magnitude(V); // Compute magnitude
    TNumber mag = popreturn(V).val_number;

    const char *keys[] = {"x", "y", "z"};
    TValue results[3];

    for (int i = 0; i < 3; ++i)
    {
        TNumber a = get_vec3_component(V, self, keys[i]).val_number;
        results[i] = stackvalue(V, a / mag);
    }

    pusharguments(V, {results[0], results[1], results[2]});
    vec3_new(V);
}

// Create vector
void vec3_new(RTState *V)
{
    TTable *vec3_ins = newtable(V, vec3_meta);
    const char *keys[] = {"x", "y", "z"};

    for (int i = 0; i < 3; ++i)
    {
        TValue val = popargument(V);
        settableindex(V, vec3_ins, hashstring(V, keys[i]), val);
    }

    pushreturn(V, stackvalue(V, vec3_ins));
}

// Predefined vectors
void vec3_one(RTState *V)
{
    pusharguments(V, {stackvalue(V, 1.0f), stackvalue(V, 1.0f), stackvalue(V, 1.0f)});
    vec3_new(V);
}

void vec3_zero(RTState *V)
{
    pusharguments(V, {stackvalue(V, 0.0f), stackvalue(V, 0.0f), stackvalue(V, 0.0f)});
    vec3_new(V);
}

// Library loader
void loadvec3lib(RTState *V)
{
    static const HashMap<const char *, TValue> vec3_meta_properties = {
        {"magnitude", stackvalue(V, vec3_magnitude)},
        {"normalize", stackvalue(V, vec3_normalize)},
        {"__add", stackvalue(V, vec3_mmadd)},
        {"__sub", stackvalue(V, vec3_mmsub)},
        {"__mul", stackvalue(V, vec3_mmmul)},
        {"__div", stackvalue(V, vec3_mmdiv)},
        {"__unm", stackvalue(V, vec3_mmunm)}
    };

    static const HashMap<const char *, TValue> vec3_gtable_properties = {
        {"new", stackvalue(V, vec3_new)},
        {"one", stackvalue(V, vec3_one)},
        {"zero", stackvalue(V, vec3_zero)},
    };

    TTable *lib = newtable(V);

    for (auto it : vec3_meta_properties)
        settableindex(V, vec3_meta, hashstring(V, it.first), it.second);

    for (auto it : vec3_gtable_properties)
        settableindex(V, lib, hashstring(V, it.first), it.second);

    freeze(V, lib);
    loadlib(V, hashstring(V, "vector3"), stackvalue(V, lib));
}

} // namespace via::lib