#pragma once

#include <cassert>
#include <new>
#include <type_traits>
#include <utility>

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
  explicit Unexpected(Args&&... args) : mUnexp(E(std::forward<Args>(args)...))
  {}

  // Move out the error value by value
  [[nodiscard]] E takeError() && { return std::move(mUnexp); }

  // If you need an lvalue overload, you must decide semantics.
  // We'll delete it to avoid accidental copies:
  [[nodiscard]] E takeError() & = delete;

 private:
  E mUnexp;
};

template <typename T>
class Expected final
{
  static_assert(!std::is_same_v<T, Error>,
                "Expected<T> cannot be instantiated with Error type");

 public:
  Expected() = delete;

  // Construct with value
  Expected(T val) noexcept(std::is_nothrow_move_constructible_v<T>)
  {
    new (&mStorage.val) T(std::move(val));
    mHasValue = true;
    mIsValid = true;
  }

  // Construct from an Unexpected<E>
  template <typename E>
  Expected(Unexpected<E>&& err) noexcept
  {
    new (&mStorage.err) Error(Error::fail<E>(std::move(err).takeError()));
    debug::require(mStorage.err.hasError(),
                   "Cannot construct Expected<T> with successful Error");
    mHasValue = false;
    mIsValid = true;
  }

  // Deleted copy
  Expected(const Expected& other) = delete;
  Expected& operator=(const Expected& other) = delete;

  // Move constructor
  Expected(Expected&& other) noexcept(
    std::is_nothrow_move_constructible_v<T>&&
      std::is_nothrow_move_constructible_v<Error>)
      : mHasValue(false), mIsValid(false)  // will be set below if we construct
  {
    if (!other.mIsValid) {
      // other is already invalid/moved-from: we remain invalid
      return;
    }

    if (other.mHasValue) {
      new (&mStorage.val) T(std::move(other.mStorage.val));
      mHasValue = true;
      mIsValid = true;
    } else {
      new (&mStorage.err) Error(std::move(other.mStorage.err));
      mHasValue = false;
      mIsValid = true;
    }

    // leave `other` in a safe (non-destroying) state
    other.destroyActive();
  }

  // Move assignment
  Expected& operator=(Expected&& other) noexcept(
    std::is_nothrow_move_constructible_v<T>&&
      std::is_nothrow_move_constructible_v<Error>)
  {
    if (this == &other)
      return *this;

    destroyActive();

    if (!other.mIsValid) {
      // other empty -> remain empty / invalid
      mIsValid = false;
      mHasValue = false;
      return *this;
    }

    if (other.mHasValue) {
      new (&mStorage.val) T(std::move(other.mStorage.val));
      mHasValue = true;
      mIsValid = true;
    } else {
      new (&mStorage.err) Error(std::move(other.mStorage.err));
      mHasValue = false;
      mIsValid = true;
    }

    other.destroyActive();
    return *this;
  }

  ~Expected() noexcept { destroyActive(); }

  operator bool() const noexcept { return hasValue(); }

  T& operator*() & noexcept { return getValue(); }
  T* operator->() noexcept { return &getValue(); }
  const T& operator*() const& noexcept { return getValue(); }
  const T* operator->() const noexcept { return &getValue(); }

 public:
  [[nodiscard]] bool hasValue() const noexcept { return mIsValid && mHasValue; }
  [[nodiscard]] bool hasError() const noexcept
  {
    return mIsValid && !mHasValue;
  }

  [[nodiscard]] T& getValue() & noexcept
  {
    debug::require(hasValue(), "Bad Expected<T> access (getValue)");
    return mStorage.val;
  }

  [[nodiscard]] const T& getValue() const& noexcept
  {
    debug::require(hasValue(), "Bad Expected<T> access (getValue)");
    return mStorage.val;
  }

  [[nodiscard]] Error& getError() & noexcept
  {
    debug::require(hasError(), "Bad Expected<T> access (getError)");
    return mStorage.err;
  }

  [[nodiscard]] const Error& getError() const& noexcept
  {
    debug::require(hasError(), "Bad Expected<T> access (getError)");
    return mStorage.err;
  }

  // Take value by value (moves out). Leaves *this invalid (no active member).
  [[nodiscard]] T takeValue() && noexcept(
    std::is_nothrow_move_constructible_v<T>)
  {
    debug::require(hasValue(), "Bad Expected<T> access (takeValue)");
    T tmp = std::move(mStorage.val);
    mStorage.val.~T();
    mIsValid = false;  // prevent destructor from trying to destroy anything
    mHasValue = false;
    return tmp;
  }

  // Take error by value (moves out). Leaves *this invalid.
  [[nodiscard]] Error takeError() && noexcept(
    std::is_nothrow_move_constructible_v<Error>)
  {
    debug::require(hasError(), "Bad Expected<T> access (takeError)");
    Error tmp = std::move(mStorage.err);
    mStorage.err.~Error();
    mIsValid = false;
    mHasValue = false;
    return tmp;
  }

  // valueOr: return copy of value or fallback
  [[nodiscard]] T valueOr(const T& orelse) const& noexcept
    requires(std::is_copy_constructible_v<T>)
  {
    return hasValue() ? getValue() : orelse;
  }

  // errorOr: return copy of error or fallback
  [[nodiscard]] Error errorOr(const Error& orelse) const& noexcept
  {
    return hasError() ? getError() : orelse;
  }

 private:
  // Storage union with non-trivial members: we manage construction/destruction
  // manually.
  union Storage
  {
    Storage() noexcept {}
    ~Storage() noexcept {}

    T val;
    Error err;
  };

  // Destroy the active member if present. After this call, object is considered
  // invalid.
  void destroyActive() noexcept
  {
    if (!mIsValid)
      return;
    if (mHasValue) {
      mStorage.val.~T();
    } else {
      mStorage.err.~Error();
    }
    mIsValid = false;
  }

 private:
  Storage mStorage;
  bool mHasValue = false;
  bool mIsValid = false;
};

}  // namespace via
