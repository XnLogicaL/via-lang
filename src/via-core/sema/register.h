// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_REGISTER_H_
#define VIA_CORE_REGISTER_H_

#include <via/config.h>
#include <util/buffer.h>

namespace via {

namespace core {

namespace sema {

struct RegisterState {
  Buffer<u64> buf{UINT16_MAX / 8};
};

int sema_alloc_register(RegisterState& S);
void sema_free_register(RegisterState& S, int reg);

} // namespace sema

} // namespace core

} // namespace via

#endif
