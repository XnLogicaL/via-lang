// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_MODULE_SYMBOL_H_
#define VIA_CORE_MODULE_SYMBOL_H_

#include <via/config.h>
#include <via/types.h>
#include <sstream>
#include "intern_table.h"

namespace via
{

using SymbolId = u64;
using QualPath = Deque<String>;

inline String to_string(const QualPath& qs)
{
  std::ostringstream oss;

  for (usize i = 0; i < qs.size(); i++) {
    if (i > 0) {
      oss << "::";
    }

    oss << qs[i];
  }

  return oss.str();
}

class SymbolTable final : public InternTable<String, SymbolId>
{
 public:
  using InternTable::intern;

 public:
  SymbolId intern(const QualPath& qs) { return intern(to_string(qs)); }
};

}  // namespace via

#endif
