// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "register.h"
#include "debug.h"

#if defined(VIA_COMPILER_GCC) || defined(VIA_COMPILER_CLANG)
  #define VIA_HAVE_BUILTIN_CTZLL
#endif

namespace via
{

namespace sema
{

namespace registers
{

static Vec<u16> regs(config::kRegisterCount / 8);

void reset()
{
  std::memset(regs.data(), 0, config::kRegisterCount / 8);
}

u16 alloc()
{
  for (u16 i = 0; i < regs.size(); i++) {
#ifdef VIA_HAVE_BUILTIN_CTZLL
    u64 word = regs[i];
    if (word != Limits<u64>::max()) {  // has at least one zero bit
      u64 inv = ~word;                 // invert zeros to ones
      i32 bit = __builtin_ctzll(inv);  // index of first zero bit
      regs[i] |= (1ULL << bit);        // mark as occupied
      return i * 64 + bit;
    }
#else
    for (u16 j = 0; j < 64; j++) {
      u64* addr = regs.data() + i;
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

void free(u16 reg)
{
  u16 word = reg / 64, bit = reg % 64;
  u64 mask = ~(1ULL << bit);
  regs[word] &= mask;  // mark bit as free
}

}  // namespace registers

}  // namespace sema

}  // namespace via
