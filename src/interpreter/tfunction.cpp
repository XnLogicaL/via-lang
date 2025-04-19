// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "tfunction.h"
#include "api-impl.h"

namespace via {

Function::Function(size_t code_size)
  : code(new Instruction[code_size]),
    code_size(code_size) {}

Function::Function(const Function& other)
  : code(new Instruction[other.code_size]),
    code_size(other.code_size),
    line_number(other.line_number),
    id(other.id) {
  std::memcpy(code, other.code, sizeof(Instruction) * code_size);
}

Function::Function(Function&& other)
  : code(other.code),
    code_size(other.code_size),
    line_number(other.line_number),
    id(other.id) {
  other.code = nullptr;
  other.code_size = 0;
  other.line_number = 0;
  other.id = "<deallocated>";
}

Function& Function::operator=(const Function& other) {
  if (this != &other) {
    this->code = new Instruction[other.code_size];
    this->code_size = other.code_size;
    this->line_number = other.line_number;
    this->id = other.id;

    std::memcpy(code, other.code, sizeof(Instruction) * code_size);
  }

  return *this;
}

Function& Function::operator=(Function&& other) {
  if (this != &other) {
    delete[] this->code;

    this->code = other.code;
    this->code_size = other.code_size;
    this->line_number = other.line_number;
    this->id = other.id;

    other.code = nullptr;
    other.code_size = 0;
    other.line_number = 0;
    other.id = "<deallocated>";
  }

  return *this;
}

Function::~Function() {
  delete[] code;
}

Callable::Callable(Function* fn, size_t arity)
  : type(Tag::Function),
    u({.fn = fn}),
    arity(arity) {}

Callable::Callable(NativeFn fn, size_t arity)
  : type(Tag::Native),
    u({.ntv = fn}),
    arity(arity) {}

Callable::Callable(const Callable& other)
  : type(other.type),
    arity(other.arity) {
  if (other.type == Tag::Function) {
    this->u.fn = new Function(*other.u.fn);
  }
  else {
    this->u = other.u;
  }
}

Callable::Callable(Callable&& other)
  : type(other.type),
    u(other.u),
    arity(other.arity) {
  other.type = Tag::None;
  other.u = {nullptr};
  other.arity = 0;
}

Callable& Callable::operator=(const Callable& other) {
  if (this != &other) {
    this->type = other.type;
    this->arity = other.arity;

    if (other.type == Tag::Function) {
      this->u.fn = new Function(*other.u.fn);
    }
    else {
      this->u = other.u;
    }
  }

  return *this;
}

Callable& Callable::operator=(Callable&& other) {
  if (this != &other) {
    delete[] this->u.fn;

    this->type = other.type;
    this->u = other.u;
    this->arity = other.arity;

    other.type = Tag::None;
    other.u = {nullptr};
    other.arity = 0;
  }

  return *this;
}

Callable::~Callable() {
  if (type == Tag::Function) {
    delete u.fn;
  }
}

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
  : upvs(new UpValue[CLOSURE_INITIAL_UPV_COUNT]) {}

Closure::~Closure() {
  delete[] upvs;
}

} // namespace via
