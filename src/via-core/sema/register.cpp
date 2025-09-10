/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "register.h"
#include <cstring>
#include <limits>
#include "debug.h"

#if defined(VIA_COMPILER_GCC) || defined(VIA_COMPILER_CLANG)
  #define VIA_HAVE_BUILTIN_CTZLL
#endif

namespace sema = via::sema;

via::u16 sema::RegisterState::alloc()
{
  for (u16 i = 0; i < mBuffer.size(); i++) {
#ifdef VIA_HAVE_BUILTIN_CTZLL
    u64 word = mBuffer[i];
    if (word != std::numeric_limits<u64>::max()) {  // has at least one zero bit
      u64 inv = ~word;                              // invert zeros to ones
      i32 bit = __builtin_ctzll(inv);               // index of first zero bit
      mBuffer[i] |= (1ULL << bit);                  // mark as occupied
      return i * 64 + bit;
    }
#else
    for (u16 j = 0; j < 64; j++) {
      u64* addr = stRegisters.data() + i;
      u64 mask = (1ULL << j);

      if ((*addr & mask) == 0ULL) {
        *addr |= mask;
        return (i * 64) + j;
      }
    }
#endif
  }

  debug::bug("semantic register allocation failure");
}

void sema::RegisterState::free(via::u16 reg)
{
  u16 word = reg / 64, bit = reg % 64;
  u64 mask = ~(1ULL << bit);
  mBuffer[word] &= mask;  // mark bit as free
}
