// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_ERROR_H_
#define VIA_CORE_ERROR_H_

#include <via/config.h>
#include <via/types.h>
#include "utility.h"

namespace via
{

class ErrorInfo
{
 public:
  ErrorInfo(std::string msg) : msg(std::move(msg)) {}

 public:
  const std::string msg;
};

class Error final
{
 public:
  Error() = default;
  Error(std::unique_ptr<ErrorInfo>&& err) noexcept : mPayload(std::move(err)) {}

  [[nodiscard]] static Error success() { return Error(); }

  operator bool() const { return !hasError(); }
  ErrorInfo& operator*() const { return getError(); }

 public:
  [[nodiscard]] bool hasError() const { return mPayload.get() != nullptr; }
  [[nodiscard]] ErrorInfo& getError() const { return *mPayload; }

 private:
  std::unique_ptr<ErrorInfo> mPayload{nullptr};
};

template <typename E = ErrorInfo, typename... Args>
[[nodiscard]] Error make_error(Args&&... args) noexcept
{
  return Error(std::make_unique<E>(fwd<Args>(args)...));
}

}  // namespace via

#endif
