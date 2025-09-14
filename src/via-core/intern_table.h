/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.h>
#include <via/types.h>
#include "option.h"

namespace via
{

template <typename T>
struct view
{
  using type = std::reference_wrapper<T>;
};

template <>
struct view<std::string>
{
  using type = std::string_view;
};

template <typename T, typename Id = u64>
class InternTable
{
 public:
  using view_type = typename view<T>::type;

 public:
  Id intern(const T& val)
  {
    auto [it, inserted] = mMap.try_emplace(val, mNextId);
    if (inserted) {
      mReverse[mNextId] = val;
      mNextId++;
    }
    return it->second;
  }

  Option<view_type> lookup(Id id) const
  {
    if (auto it = mReverse.find(id); it != mReverse.end()) {
      return view_type(it->second);
    }
    return nullopt;
  }

 protected:
  usize mNextId = 0;
  std::unordered_map<T, Id> mMap;
  std::unordered_map<Id, T> mReverse;
};

}  // namespace via
