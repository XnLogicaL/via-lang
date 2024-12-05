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

viaValue get_vec3_component(viaState *V, viaTable *vec, viaRawString_t key);
template<typename Op>
void vec3_binary_op(viaState *V, Op op);
// Utility: Perform a unary operation on a vector
template<typename Op>
void vec3_unary_op(viaState *V, Op op);
// Vector3 operations
void vec3_mmadd(viaState *V);
void vec3_mmsub(viaState *V);
void vec3_mmmul(viaState *V);
void vec3_mmdiv(viaState *V);
void vec3_mmunm(viaState *V);
// Magnitude function
void vec3_magnitude(viaState *V);
// Normalize vector3
void vec3_normalize(viaState *V);
// Vector3 constructor
void vec3_new(viaState *V);
// Predefined vectors
void vec3_one(viaState *V);
void vec3_zero(viaState *V);
// Library loader
void viaL_loadvec3lib(viaState *V);

} // namespace via::lib
