/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vlmath.h"

namespace via::lib
{

void math_exp(RTState *V)
{
    TValue *num = getargument(V, 0);

    LIB_ASSERT(checknumber(V, *num), ARG_MISMATCH(0, "Number", ENUM_NAME(num->type)));

    TValue val = stackvalue(V, std::exp(num->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_log(RTState *V)
{
    TValue *base = getargument(V, 0);
    TValue *num = getargument(V, 1);

    LIB_ASSERT(checknumber(V, *base), ARG_MISMATCH(0, "Number", ENUM_NAME(base->type)));
    LIB_ASSERT(checknumber(V, *num), ARG_MISMATCH(1, "Number", ENUM_NAME(num->type)));

    TValue val = stackvalue(V, std::log(num->val_number) / std::log(base->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_log10(RTState *V)
{
    TValue *num = getargument(V, 0);

    LIB_ASSERT(checknumber(V, *num), ARG_MISMATCH(0, "Number", ENUM_NAME(num->type)));

    TValue val = stackvalue(V, std::log10(num->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_pow(RTState *V)
{
    TValue *num = getargument(V, 0);
    TValue *e = getargument(V, 1);

    LIB_ASSERT(checknumber(V, *num), ARG_MISMATCH(0, "Number", ENUM_NAME(num->type)));
    LIB_ASSERT(checknumber(V, *e), ARG_MISMATCH(1, "Number", ENUM_NAME(e->type)));

    TValue val = stackvalue(V, std::pow(num->val_number, e->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_cos(RTState *V)
{
    TValue *t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, *t), ARG_MISMATCH(0, "Number", ENUM_NAME(t->type)));

    TValue val = stackvalue(V, std::cos(t->val_number)); // Compute the cosine
    pushval(V, val);
    nativeret(V, 1);
}

void math_tan(RTState *V)
{
    TValue *t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, *t), ARG_MISMATCH(0, "Number", ENUM_NAME(t->type)));

    TValue val = stackvalue(V, std::tan(t->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_asin(RTState *V)
{
    TValue *t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, *t), ARG_MISMATCH(0, "Number", ENUM_NAME(t->type)));

    TValue val = stackvalue(V, std::asin(t->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_acos(RTState *V)
{
    TValue *t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, *t), ARG_MISMATCH(0, "Number", ENUM_NAME(t->type)));

    TValue val = stackvalue(V, std::acos(t->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_atan(RTState *V)
{
    TValue *t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, *t), ARG_MISMATCH(0, "Number", ENUM_NAME(t->type)));

    TValue val = stackvalue(V, std::atan(t->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_atan2(RTState *V)
{
    TValue *t = getargument(V, 0);
    TValue *x = getargument(V, 1);

    LIB_ASSERT(checknumber(V, *t), ARG_MISMATCH(0, "Number", ENUM_NAME(t->type)));
    LIB_ASSERT(checknumber(V, *x), ARG_MISMATCH(1, "Number", ENUM_NAME(x->type)));

    TValue val = stackvalue(V, std::atan2(t->val_number, x->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_sinh(RTState *V)
{
    TValue *t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, *t), ARG_MISMATCH(0, "Number", ENUM_NAME(t->type)));

    TValue val = stackvalue(V, std::sinh(t->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_cosh(RTState *V)
{
    TValue *t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, *t), ARG_MISMATCH(0, "Number", ENUM_NAME(t->type)));

    TValue val = stackvalue(V, std::cosh(t->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_tanh(RTState *V)
{
    TValue *t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, *t), ARG_MISMATCH(0, "Number", ENUM_NAME(t->type)));

    TValue val = stackvalue(V, std::tanh(t->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_abs(RTState *V)
{
    TValue *t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, *t), ARG_MISMATCH(0, "Number", ENUM_NAME(t->type)));

    TValue val = stackvalue(V, std::fabs(t->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_min(RTState *V)
{
    TValue *t = getargument(V, 0);
    TValue *x = getargument(V, 1);

    LIB_ASSERT(checknumber(V, *t), ARG_MISMATCH(0, "Number", ENUM_NAME(t->type)));
    LIB_ASSERT(checknumber(V, *x), ARG_MISMATCH(1, "Number", ENUM_NAME(x->type)));

    TValue val = stackvalue(V, std::min(t->val_number, x->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_max(RTState *V)
{
    TValue *t = getargument(V, 0);
    TValue *x = getargument(V, 1);

    LIB_ASSERT(checknumber(V, *t), ARG_MISMATCH(0, "Number", ENUM_NAME(t->type)));
    LIB_ASSERT(checknumber(V, *x), ARG_MISMATCH(1, "Number", ENUM_NAME(x->type)));

    TValue val = stackvalue(V, std::max(t->val_number, x->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_round(RTState *V)
{
    TValue *t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, *t), ARG_MISMATCH(0, "Number", ENUM_NAME(t->type)));

    TValue val = stackvalue(V, std::round(t->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_floor(RTState *V)
{
    TValue *t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, *t), ARG_MISMATCH(0, "Number", ENUM_NAME(t->type)));

    TValue val = stackvalue(V, std::floor(t->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void math_ceil(RTState *V)
{
    TValue *t = getargument(V, 0);

    LIB_ASSERT(checknumber(V, *t), ARG_MISMATCH(0, "Number", ENUM_NAME(t->type)));

    TValue val = stackvalue(V, std::ceil(t->val_number));
    pushval(V, val);
    nativeret(V, 1);
}

void loadmathlib(RTState *V)
{
    static const HashMap<const char *, TValue> math_properties = {

        // Constants
        {"pi", stackvalue(V, 3.1415926535)},

        // Functions
        {"exp", WRAPVAL(math_exp)},
        {"log", WRAPVAL(math_log)},
        {"log10", WRAPVAL(math_log10)},
        {"pow", WRAPVAL(math_pow)},
        {"cos", WRAPVAL(math_cos)},
        {"tan", WRAPVAL(math_tan)},
        {"asin", WRAPVAL(math_asin)},
        {"acos", WRAPVAL(math_acos)},
        {"atan", WRAPVAL(math_atan)},
        {"atan2", WRAPVAL(math_atan2)},
        {"sinh", WRAPVAL(math_sinh)},
        {"cosh", WRAPVAL(math_cosh)},
        {"tanh", WRAPVAL(math_tanh)},
        {"abs", WRAPVAL(math_abs)},
        {"min", WRAPVAL(math_min)},
        {"max", WRAPVAL(math_max)},
        {"round", WRAPVAL(math_round)},
        {"floor", WRAPVAL(math_floor)},
        {"ceil", WRAPVAL(math_ceil)}
    };

    TTable *lib = newtable(V, nullptr, {});

    for (auto it : math_properties)
    {
        TableKey key = hashstring(V, it.first);
        settableindex(V, lib, key, it.second);
    }

    freeze(V, lib);
}

} // namespace via::lib