// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_OPTION_H_
#define VIA_CORE_OPTION_H_

#include <via/config.h>
#include <via/types.h>
#include "utility.h"

namespace via
{

struct Nullopt
{};

inline constexpr Nullopt nullopt;

template <typename T>
class Option final
{
 public:
  union Storage
  {
    T val;
    Nullopt null;

    Storage() : null() {}
    Storage(T&& val) : val(mv(val)) {}
    Storage(const Storage&) {}
    Storage(Storage&&) {}
    ~Storage() {}
  };

 public:
  constexpr Option(Nullopt) noexcept : mHasValue(false), mStorage() {}
  constexpr Option(T val) noexcept : mHasValue(true), mStorage(fwd<T>(val)) {}

  constexpr ~Option() noexcept
  {
    if (hasValue()) {
      mStorage.val.~T();
    }
  }

  constexpr operator bool() const noexcept { return hasValue(); }
  constexpr T& operator*() noexcept { return getValue(); }
  constexpr T* operator->() noexcept { return &getValue(); }
  constexpr const T& operator*() const noexcept { return getValue(); }
  constexpr const T* operator->() const noexcept { return &getValue(); }

 public:
  [[nodiscard]] constexpr bool hasValue() const noexcept { return mHasValue; }

  [[nodiscard]] constexpr T& getValue() noexcept { return mStorage.val; }
  [[nodiscard]] constexpr const T& getValue() const noexcept
  {
    return mStorage.val;
  }

 private:
  Storage mStorage;
  bool mHasValue;
};

}  // namespace via

#endif
