/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vlrand.h"
#include "mathutils.h"

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
TNumber pcg32_range(TNumber a, TNumber b)
{
    // Get a random number in the full uint64_t range
    uint64_t rand_val = pcg32_rand();
    // Map it to the range [0.0, 1.0]
    TNumber scaled = static_cast<TNumber>(rand_val) / static_cast<TNumber>(UINT64_MAX);
    // Now scale it to the desired range [a, b]
    return lerp(a, b, scaled);
}

void rand_range(RTState *V)
{
    TValue low = getargument(V, 0);
    TValue high = getargument(V, 1);
    TValue num = TValue(pcg32_range(low.val_number, high.val_number));

    pushval(V, num);
    nativeret(V, 1);
}

void rand_int(RTState *V)
{
    rand_range(V);
    TValue x = popval(V);
    x.val_number = std::floor(x.val_number);

    pushval(V, x);
    nativeret(V, 1);
}

void loadrandlib(RTState *V)
{
    static const HashMap<const char *, TValue> rand_properties = {
        {"range", TValue(new TCFunction(rand_range))},
        {"int", TValue(new TCFunction(rand_int))},
    };

    TTable *lib = new TTable();

    for (auto it : rand_properties)
    {
        TableKey key = hashstring(V, it.first);
        settable(V, lib, key, it.second);
    }

    freeze(V, lib);
}

} // namespace via::lib