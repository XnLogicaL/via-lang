// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_INTERN_TABLE_H_
#define VIA_CORE_INTERN_TABLE_H_

#include <via/config.h>
#include <via/types.h>

namespace via
{

template <typename T>
struct View
{
  using type = Ref<T>;
};

template <>
struct View<String>
{
  using type = StringView;
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
    if (inserted)
      mNextId++;
    return it->second;
  }

  Optional<View> lookup(Id id) const
  {
    if (auto it = mMap.find(id))
      return it->first;
    return nullopt;
  }

 protected:
  usize mNextId = 0;
  Map<T, Id> mMap;
};

}  // namespace via

#endif
