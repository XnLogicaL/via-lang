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

static TTable *vec3_meta = new TTable;

TValue get_vec3_component(RTState *V, TTable *vec, const char *key);
template<typename Op>
void vec3_binary_op(RTState *V, Op op);
// Utility: Perform a unary operation on a vector
template<typename Op>
void vec3_unary_op(RTState *V, Op op);
// Vector3 operations
void vec3_mmadd(RTState *V);
void vec3_mmsub(RTState *V);
void vec3_mmmul(RTState *V);
void vec3_mmdiv(RTState *V);
void vec3_mmunm(RTState *V);
// Magnitude function
void vec3_magnitude(RTState *V);
// Normalize vector3
void vec3_normalize(RTState *V);
// Vector3 constructor
void vec3_new(RTState *V);
// Predefined vectors
void vec3_one(RTState *V);
void vec3_zero(RTState *V);
// Library loader
void loadvec3lib(RTState *V);

} // namespace via::lib
