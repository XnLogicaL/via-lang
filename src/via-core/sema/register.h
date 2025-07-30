// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_REGISTER_H_
#define VIA_CORE_REGISTER_H_

#include <util/buffer.h>
#include <via/config.h>

namespace via {

namespace core {

namespace sema {

struct RegisterState {
  Buffer<u64> buf{UINT16_MAX / 8};
};

int alloc_register(RegisterState& S);
void free_register(RegisterState& S, int reg);

}  // namespace sema

}  // namespace core

}  // namespace via

#endif
