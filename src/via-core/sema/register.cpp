// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "register.h"
#include "debug.h"

#if defined(VIA_COMPILER_GCC) || defined(VIA_COMPILER_CLANG)
  #define VIA_HAVE_BUILTIN_CTZLL
#endif

namespace sema = via::sema;

static via::Vec<via::u16> stRegisters(via::config::kRegisterCount / 8);

void sema::registers::reset()
{
  std::memset(stRegisters.data(), 0, via::config::kRegisterCount / 8);
}

via::u16 sema::registers::alloc()
{
  for (u16 i = 0; i < stRegisters.size(); i++) {
#ifdef VIA_HAVE_BUILTIN_CTZLL
    u64 word = stRegisters[i];
    if (word != Limits<u64>::max()) {   // has at least one zero bit
      u64 inv = ~word;                  // invert zeros to ones
      i32 bit = __builtin_ctzll(inv);   // index of first zero bit
      stRegisters[i] |= (1ULL << bit);  // mark as occupied
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

void sema::registers::free(via::u16 reg)
{
  u16 word = reg / 64, bit = reg % 64;
  u64 mask = ~(1ULL << bit);
  stRegisters[word] &= mask;  // mark bit as free
}
