// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "register.h"
#include "debug.h"

#if defined(VIA_COMPILER_GCC) || defined(VIA_COMPILER_CLANG)
#define VIA_HAVE_BUILTIN_CTZLL
#endif

namespace via {

namespace sema {

u16 alloc_register(Context& ctx) {
  for (u16 i = 0; i < ctx.regs.size(); i++) {
#ifdef VIA_HAVE_BUILTIN_CTZLL
    u64 word = ctx.regs[i];
    if (word != Limits<u64>::max()) {  // has at least one zero bit
      u64 inv = ~word;                 // invert zeros to ones
      i32 bit = __builtin_ctzll(inv);  // index of first zero bit
      ctx.regs[i] |= (1ULL << bit);    // mark as occupied
      return i * 64 + bit;
    }
#else
    for (u16 j = 0; j < 64; j++) {
      u64* addr = ctx.regs.data() + i;
      u64 mask = (1ULL << j);

      if ((*addr & mask) == 0ULL) {
        *addr |= mask;
        return (i * 64) + j;
      }
    }
#endif
  }

  bug("semantic register allocation failure");
  std::unreachable();
}

void free_register(Context& ctx, u16 reg) {
  u16 word = reg / 64, bit = reg % 64;
  u64 mask = ~(1ULL << bit);
  ctx.regs[word] &= mask;  // mark bit as free
}

}  // namespace sema

}  // namespace via
