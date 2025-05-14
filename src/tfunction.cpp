// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "tfunction.h"
#include "api-impl.h"

namespace via {

Closure::Closure(Callable&& callable)
  : callee(std::move(callable)) {}

Closure::Closure(const Closure& other)
  : callee(other.callee),
    upvs(new UpValue[other.upv_count]),
    upv_count(other.upv_count) {
  // UpValues captured twice; close them.
  impl::__closure_close_upvalues(&other);

  for (size_t i = 0; i < upv_count; i++) {
    UpValue& upv = this->upvs[i];
    UpValue& other_upv = other.upvs[i];
    upv.heap_value = other_upv.heap_value.clone();
    upv.value = &upv.heap_value;
    upv.is_valid = true;
    upv.is_open = false;
  }
}

Closure::Closure(Closure&& other)
  : callee(other.callee),
    upvs(other.upvs),
    upv_count(other.upv_count) {
  // Only reset upvalues because they are the only owned values.
  other.upvs = nullptr;
  other.upv_count = 0;
}

Closure& Closure::operator=(const Closure& other) {
  if (this != &other) {
    this->callee = other.callee;
    this->upvs = new UpValue[other.upv_count];
    this->upv_count = other.upv_count;

    // UpValues captured twice; close them.
    impl::__closure_close_upvalues(&other);

    for (size_t i = 0; i < upv_count; i++) {
      UpValue& upv = this->upvs[i];
      UpValue& other_upv = other.upvs[i];
      upv.heap_value = other_upv.heap_value.clone();
      upv.value = &upv.heap_value;
      upv.is_valid = true;
      upv.is_open = false;
    }
  }

  return *this;
}

Closure& Closure::operator=(Closure&& other) {
  if (this != &other) {
    delete[] this->upvs;

    this->callee = other.callee;
    this->upvs = other.upvs;
    this->upv_count = other.upv_count;
    // Only reset upvalues because they are the only owned values.
    other.upvs = nullptr;
    other.upv_count = 0;
  }

  return *this;
}

Closure::Closure()
  : upvs(new UpValue[CLOSURE_INITIAL_UPV_COUNT]),
    upv_count(CLOSURE_INITIAL_UPV_COUNT) {}

Closure::~Closure() {
  delete[] upvs;
}

} // namespace via
