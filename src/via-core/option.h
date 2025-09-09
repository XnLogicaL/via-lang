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
#include "debug.h"
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
    debug::require(hasValue(), "Bad Option<T> access");
    return mStorage.val;
  }

  [[nodiscard]] constexpr T&& takeValue() noexcept
  {
    debug::require(hasValue(), "Bad Option<T> access");
    mHasValue = false;
    return std::move(mStorage.val);
  }

  [[nodiscard]] constexpr T valueOr(T orelse) const noexcept
    requires(std::is_copy_constructible_v<T>)
  {
    return hasValue() ? getValue() : orelse;
  }

 private:
  Storage mStorage;
  bool mHasValue;
};

}  // namespace via
