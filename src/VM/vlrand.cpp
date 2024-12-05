/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vlrand.h"

namespace via::lib
{

uint64_t pcg32_rand()
{
    static uint64_t seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    uint64_t oldstate = seed;
    seed = oldstate * 6364136223846793005ULL + 1;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

// Function to generate a random double in the range [a, b]
viaNumber pcg32_range(viaNumber a, viaNumber b)
{
    // Get a random number in the full uint64_t range
    uint64_t rand_val = pcg32_rand();
    // Map it to the range [0.0, 1.0]
    viaNumber scaled = static_cast<viaNumber>(rand_val) / static_cast<viaNumber>(UINT64_MAX);
    // Now scale it to the desired range [a, b]
    return a + scaled * (b - a);
}

void rand_range(viaState *V)
{
    viaValue low = via_popargument(V);
    viaValue high = via_popargument(V);

    viaValue num = viaT_stackvalue(V, pcg32_range(low.val_number, high.val_number));

    via_pushreturn(V, num);
}

void rand_int(viaState *V)
{
    rand_range(V);
    viaValue x = via_popreturn(V);
    x.val_number = std::floor(x.val_number);
}

void viaL_loadrandlib(viaState *V)
{
    static const viaHashMap_t<viaRawString_t, viaValue> rand_properties = {
        {"range", viaT_stackvalue(V, rand_range)},
        {"int", viaT_stackvalue(V, rand_int)},
    };

    viaTable *lib = viaT_newtable(V);

    for (auto it : rand_properties)
    {
        viaTableKey key = viaT_hashstring(V, it.first);
        via_settableindex(V, lib, key, it.second);
    }

    via_freeze(V, lib);
    via_loadlib(V, viaT_hashstring(V, "random"), viaT_stackvalue(V, lib));
}

} // namespace via::lib