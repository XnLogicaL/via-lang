/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vlrand.h"

namespace via::lib
{

template<typename T>
    requires std::is_arithmetic_v<T>
VIA_INLINE T lerp(T a, T b, float t)
{
    return a + (b - a) * t;
}

uint64_t pcg32_rand()
{
    static uint64_t seed = std::chrono::steady_clock::now().time_since_epoch().count();
    uint64_t oldstate = seed;
    seed = oldstate * 6364136223846793005ULL + 1;
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

// Function to generate a random double in the range [a, b]
TNumber pcg32_range(TNumber a, TNumber b)
{
    uint64_t rand_val = pcg32_rand();
    TNumber scaled = static_cast<TNumber>(rand_val) / static_cast<TNumber>(UINT64_MAX);
    return lerp(a, b, scaled);
}

} // namespace via::lib