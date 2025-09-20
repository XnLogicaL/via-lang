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
#include "support/utility.h"

namespace via {

class Error;
class ErrorInfo
{
  public:
    ErrorInfo(const Error& err);
    ErrorInfo(std::string msg)
        : msg(std::move(msg))
    {}

  public:
    const std::string msg;
};

class Error final
{
  public:
    Error() = default;
    Error(std::shared_ptr<ErrorInfo>&& err) noexcept
        : m_payload(std::move(err))
    {}

    [[nodiscard]] static Error success() noexcept { return Error(); }

    template <typename E = ErrorInfo, typename... Args>
    [[nodiscard]] static Error fail(Args&&... args) noexcept
    {
        return Error(std::make_shared<E>(forward<Args>(args)...));
    }

    operator bool() const noexcept { return !has_error(); }
    ErrorInfo& operator*() const noexcept { return get_error(); }

  public:
    [[nodiscard]] bool has_error() const noexcept { return m_payload.get(); }
    [[nodiscard]] ErrorInfo& get_error() const noexcept { return *m_payload; }
    [[nodiscard]] std::string to_string() const noexcept { return m_payload->msg; }

  private:
    std::shared_ptr<ErrorInfo> m_payload{nullptr};
};

inline ErrorInfo::ErrorInfo(const Error& err)
    : msg(err.get_error().msg)
{}

} // namespace via
