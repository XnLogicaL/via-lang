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
#include "utility.h"

namespace via
{

class Error;
class ErrorInfo
{
 public:
  ErrorInfo(std::string msg) : msg(std::move(msg)) {}
  ErrorInfo(const Error& err);

 public:
  const std::string msg;
};

class Error final
{
 public:
  Error() = default;
  Error(std::shared_ptr<ErrorInfo>&& err) noexcept : mPayload(std::move(err)) {}

  [[nodiscard]] static Error success() noexcept { return Error(); }

  template <typename E = ErrorInfo, typename... Args>
  [[nodiscard]] static Error fail(Args&&... args) noexcept
  {
    return Error(std::make_shared<E>(fwd<Args>(args)...));
  }

  operator bool() const noexcept { return !hasError(); }
  ErrorInfo& operator*() const noexcept { return getError(); }

 public:
  [[nodiscard]] bool hasError() const noexcept { return mPayload.get(); }
  [[nodiscard]] ErrorInfo& getError() const noexcept { return *mPayload; }
  [[nodiscard]] std::string toString() const noexcept { return mPayload->msg; }

 private:
  std::shared_ptr<ErrorInfo> mPayload{nullptr};
};

inline ErrorInfo::ErrorInfo(const Error& err) : msg(err.getError().msg) {}

}  // namespace via
