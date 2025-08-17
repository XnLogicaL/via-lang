// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_SEMA_STACK_H_
#define VIA_CORE_SEMA_STACK_H_

#include <via/config.h>
#include <via/types.h>
#include "variable.h"

namespace via {

namespace sema {

class Frame final {
 public:
  Vec<Local>& get_locals() { return locals; }
  Optional<LocalRef> find_local(String symbol);

 private:
  Vec<Local> locals;
};

class Stack final {
 public:
  Frame& top() { return frames.back(); }
  const Frame& top() const { return frames.back(); }

  void push(Frame&& frame) { frames.push_back(std::move(frame)); }

 private:
  Vec<Frame> frames;
};

}  // namespace sema

}  // namespace via

#endif
