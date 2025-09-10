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
#include "error.h"

namespace via
{

template <typename E = ErrorInfo>
class Unexpected final
{
 public:
  Unexpected()
    requires(std::is_default_constructible_v<E>)
  = default;

  template <typename... Args>
    requires(std::is_constructible_v<E, Args...>)
  Unexpected(Args&&... args) : mUnexp(fwd<Args>(args)...)
  {}

 public:
  [[nodiscard]] E&& takeError() { return mv(mUnexp); }

 private:
  E mUnexp;
};

template <typename T>
class Expected final
{
 public:
  union Storage
  {
    T val;
    Error err;

    constexpr Storage(T&& val) noexcept : val(mv(val)) {}
    constexpr Storage(Error&& err) noexcept : err(mv(err)) {}
    constexpr ~Storage() noexcept {}
  };

 public:
  constexpr Expected(T val) noexcept : mStorage(fwd<T>(val)), mHasValue(true) {}

  template <typename E>
  constexpr Expected(Unexpected<E>&& err) noexcept
      : mStorage(fwd<Error>(Error::fail<E>(err.takeError()))), mHasValue(false)
  {
    debug::require(mStorage.err.hasError(),
                   "Cannot construct Expected<T> with successful Error");
  }

  constexpr Expected(const Expected& other) = delete;

  constexpr Expected(Expected&& other)
      : mHasValue(other.hasValue()), mStorage(other.mTakeStorage())
  {
    // UB if we don't do this lol
    other.mIsValid = false;
  }

  constexpr ~Expected() noexcept
  {
    if (!mIsValid)
      return;

    if (mHasValue) {
      mStorage.val.~T();
    } else {
      mStorage.err.~Error();
    }
  }

  constexpr Expected& operator=(const Expected& other) = delete;
  constexpr Expected& operator=(Expected&& other)
  {
    if (this != other) {
      if (other.hasValue()) {
        mHasValue = true;
        mStorage.val = other.takeValue();
      } else {
        mHasValue = false;
        mStorage.err = other.takeError();
      }

      // UB if we don't do this lol
      other.mIsValid = false;
    }

    return *this;
  }

  constexpr operator bool() const noexcept { return hasValue(); }
  constexpr T& operator*() noexcept { return getValue(); }
  constexpr T* operator->() noexcept { return &getValue(); }
  constexpr const T& operator*() const noexcept { return getValue(); }
  constexpr const T* operator->() const noexcept { return &getValue(); }

 public:
  [[nodiscard]] constexpr bool hasValue() const noexcept { return mHasValue; }
  [[nodiscard]] constexpr bool hasError() const noexcept { return !mHasValue; }

  [[nodiscard]] constexpr T& getValue() noexcept
  {
    debug::require(hasValue(), "Bad Expected<T> access (getValue)");
    return mStorage.val;
  }

  [[nodiscard]] constexpr const T& getValue() const noexcept
  {
    debug::require(hasValue(), "Bad Expected<T> access (getValue)");
    return mStorage.val;
  }

  [[nodiscard]] constexpr T valueOr(T orelse) const noexcept
    requires(std::is_copy_constructible_v<T>)
  {
    return hasValue() ? getValue() : orelse;
  }

  [[nodiscard]] constexpr T&& takeValue() noexcept
  {
    debug::require(hasValue(), "Bad Expected<T> access (takeValue)");
    mHasValue = false;
    return mv(mStorage.val);
  }

  [[nodiscard]] constexpr Error& getError() noexcept
  {
    debug::require(hasError(), "Bad Expected<T> access (getError)");
    return mStorage.err;
  }

  [[nodiscard]] constexpr const Error& getError() const noexcept
  {
    debug::require(hasError(), "Bad Expected<T> access (getError)");
    return mStorage.err;
  }

  [[nodiscard]] constexpr Error errorOr(Error orelse) const noexcept
  {
    return hasError() ? getError() : orelse;
  }

  [[nodiscard]] constexpr Error&& takeError() noexcept
  {
    debug::require(hasError(), "Bad Expected<T> access (takeError)");
    mHasValue = true;
    return mv(mStorage.err);
  }

 private:
  [[nodiscard]] constexpr Storage&& mTakeStorage() { return mv(mStorage); }

 private:
  Storage mStorage;
  bool mHasValue;

  // TODO: Get rid of this when move constructors/operators no longer call the
  // destructor of the moved value
  bool mIsValid = true;
};

}  // namespace via
