// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |
#include "vlrand.h"

namespace via::lib {

template<typename T>
    requires std::is_arithmetic_v<T>
VIA_INLINE T lerp(T a, T b, float t)
{
    return a + (b - a) * t;
}

U64 pcg32_rand()
{
    static U64 seed = std::chrono::steady_clock::now().time_since_epoch().count();
    U64 oldstate = seed;
    seed = oldstate * 6364136223846793005ULL + 1;
    U32 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    U32 rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

// Function to generate a random double in the range [a, b]
TNumber pcg32_range(TNumber a, TNumber b)
{
    U64 rand_val = pcg32_rand();
    TNumber scaled = static_cast<TNumber>(rand_val) / static_cast<TNumber>(UINT64_MAX);
    return lerp(a, b, scaled);
}

} // namespace via::lib