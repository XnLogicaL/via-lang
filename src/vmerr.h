// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_ERR_H
#define VIA_ERR_H

#include <csetjmp>
#include "common.h"

namespace via {

namespace vm {

struct State;

struct ErrorContext {
  jmp_buf env;
  const char* msg;
  ErrorContext* prev;
};

void error_fatal(const char* msg);

void error(State* S, const char* msg);
void error_toobig(State* S);
void error_outofbounds(State* S);

} // namespace vm

} // namespace via

#endif
