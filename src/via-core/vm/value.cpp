// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "value.h"

namespace via {

namespace core {

namespace vm {

void Value::free() {}

ValueRef Value::make_ref() {
  return ValueRef(ctx, this);
}

}  // namespace vm

}  // namespace core

}  // namespace via
