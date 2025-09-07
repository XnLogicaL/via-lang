// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_MODULE_SYMBOL_H_
#define VIA_CORE_MODULE_SYMBOL_H_

#include <via/config.h>
#include <via/types.h>
#include <mutex>
#include <sstream>
#include "intern_table.h"

namespace via
{

using SymbolId = u64;
using QualPath = Deque<std::string>;

inline std::string toString(const QualPath& path)
{
  std::ostringstream oss;

  for (usize i = 0; i < path.size(); i++) {
    if (i > 0) {
      oss << "::";
    }

    oss << path[i];
  }

  return oss.str();
}

class SymbolTable final : public InternTable<std::string, SymbolId>
{
 public:
  using InternTable::intern;

  static SymbolTable& instance()
  {
    static SymbolTable inst;
    return inst;
  }

  const auto& getSymbols() const { return mMap; }

  SymbolId intern(const QualPath& path)
  {
    std::lock_guard<std::mutex> lock(mMutex);
    return intern(toString(path));
  }

 private:
  mutable std::mutex mMutex;
};

}  // namespace via

#endif
