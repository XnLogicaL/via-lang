// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_RESULT_H_
#define VIA_CORE_RESULT_H_

#include <via/config.h>
#include <via/types.h>
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

    Storage(T&& val) noexcept : val(mv(val)) {}
    Storage(Error&& err) noexcept : err(mv(err)) {}
    ~Storage() noexcept {}
  };

 public:
  Expected(T&& val) noexcept : mStorage(fwd<T>(val)), mHasValue(true) {}

  template <typename E>
  Expected(Unexpected<E>&& err) noexcept
      : mStorage(fwd<Error>(make_error<E>(err.takeError()))), mHasValue(false)
  {
    debug::assertm(mStorage.err.hasError(),
                   "Cannot construct Expected<T> with successful Error");
  }

  Expected(const Expected& other) = delete;

  Expected(Expected&& other)
      : mHasValue(other.hasValue()), mStorage(other.mTakeStorage())
  {
    // UB if we don't do this lol
    other.mIsValid = false;
  }

  ~Expected() noexcept
  {
    if (!mIsValid)
      return;

    if (mHasValue) {
      mStorage.val.~T();
    } else {
      mStorage.err.~Error();
    }
  }

  Expected& operator=(const Expected& other) = delete;
  Expected& operator=(Expected&& other)
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

  operator bool() const noexcept { return hasValue(); }
  T& operator*() const noexcept { return getValue(); }
  T* operator->() const noexcept { return &getValue(); }

 public:
  [[nodiscard]] bool hasValue() const noexcept { return mHasValue; }
  [[nodiscard]] bool hasError() const noexcept { return !mHasValue; }

  [[nodiscard]] T& getValue() const noexcept
  {
    debug::assertm(hasValue(), "Bad Expected<T> access (getValue)");
    return mStorage.val;
  }

  [[nodiscard]] T&& takeValue() noexcept
  {
    debug::assertm(hasValue(), "Bad Expected<T> access (takeValue)");
    mHasValue = false;
    return mv(mStorage.val);
  }

  [[nodiscard]] ErrorInfo& getError() const noexcept
  {
    debug::assertm(hasError(), "Bad Expected<T> access (getError)");
    return mStorage.err;
  }

  [[nodiscard]] ErrorInfo&& takeError() noexcept
  {
    debug::assertm(hasError(), "Bad Expected<T> access (takeError)");
    mHasValue = true;
    return mv(mStorage.err);
  }

 private:
  [[nodiscard]] Storage&& mTakeStorage() { return mv(mStorage); }

 private:
  Storage mStorage;
  bool mHasValue;

  // TODO: Get rid of this when move constructors/operators no longer call the
  // destructor of the moved value
  bool mIsValid = true;
};

}  // namespace via

#endif
