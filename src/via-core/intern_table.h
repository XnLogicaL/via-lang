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
struct View
{
  using type = std::reference_wrapper<T>;
};

template <>
struct View<std::string>
{
  using type = std::string_view;
};

template <typename T, typename Id = u64>
class InternTable
{
 public:
  using View = View<T>::type;

 public:
  Id intern(const T& t)
  {
    auto [it, inserted] = mMap.try_emplace(t, mNextId);
    if (inserted) {
      mReverse[mNextId] = t;
      mNextId++;
    }
    return it->second;
  }

  Option<View> lookup(Id id) const
  {
    if (auto it = mReverse.find(id); it != mReverse.end())
      return it->second;
    return nullopt;
  }

 protected:
  usize mNextId = 0;
  Map<T, Id> mMap;
  Map<Id, T> mReverse;
};

}  // namespace via
