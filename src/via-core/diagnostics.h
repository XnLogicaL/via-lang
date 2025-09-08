/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <via/config.h>
#include <via/types.h>
#include "lexer/location.h"
#include "utility.h"

namespace via
{

struct Diagnosis
{
  enum class Kind : u8
  {
    Info,
    Warn,
    Error,
  };

  Kind kind = Kind::Info;
  SourceLoc loc;    // Absolute location in the source buffer
  std::string msg;  // Human-readable message
};

class DiagContext final
{
 public:
  DiagContext(std::string path, std::string name, const std::string& source)
      : mPath(path), mName(name), mSource(source)
  {}

  NO_COPY(DiagContext)

 public:
  void emit(spdlog::logger* logger = spdlog::default_logger().get()) const;
  void clear() noexcept { mDiags.clear(); }

  template <Diagnosis::Kind K>
  void report(SourceLoc loc, std::string msg)
  {
    mDiags.emplace_back(K, loc, std::move(msg));
  }

  /// Push a diagnosis using fmt-style formatting.
  template <Diagnosis::Kind K, typename... Args>
  void report(SourceLoc loc, fmt::format_string<Args...> fmt, Args&&... args)
  {
    mDiags.emplace_back(
      K, loc,
      fmt::format(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
  }

  [[nodiscard]] Vec<Diagnosis>& diagnostics() noexcept { return mDiags; }
  [[nodiscard]] const Vec<Diagnosis>& diagnostics() const noexcept
  {
    return mDiags;
  }

  [[nodiscard]] bool hasErrors() const noexcept
  {
    for (const auto& d : mDiags) {
      if (d.kind == Diagnosis::Kind::Error)
        return true;
    }

    return false;
  }

  [[nodiscard]] const std::string& source() const noexcept { return mSource; }

 private:
  // Helper to pretty-print a single diagnosis line with source context.
  void emitOnce(const Diagnosis& d, spdlog::logger* logger) const;

 private:
  std::string mPath, mName;
  const std::string& mSource;
  Vec<Diagnosis> mDiags{};
};

}  // namespace via
