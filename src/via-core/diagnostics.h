// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_DIAG_H_
#define VIA_CORE_DIAG_H_

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <via/config.h>
#include <via/types.h>
#include "lexer/lexer.h"
#include "lexer/location.h"

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
  SourceLoc loc;  // Absolute location in the source buffer
  String msg;     // Human-readable message
};

class DiagContext final
{
 public:
  DiagContext(String path, String name, const String& file)
      : m_path(path), m_name(name), m_source(file)
  {}

  NO_COPY(DiagContext)

 public:
  /// Emit all queued diagnostics to the provided spdlog logger (or default).
  void emit(spdlog::logger* logger = spdlog::default_logger().get()) const;

  /// Remove all queued diagnostics.
  void clear() noexcept { m_diags.clear(); }

  /// Push a diagnosis with a pre-formatted message.
  template <Diagnosis::Kind K>
  void report(SourceLoc loc, String msg)
  {
    m_diags.emplace_back(K, loc, std::move(msg));
  }

  /// Push a diagnosis using fmt-style formatting.
  template <Diagnosis::Kind K, typename... Args>
  void report(SourceLoc loc, fmt::format_string<Args...> fmt, Args&&... args)
  {
    m_diags.emplace_back(
        K, loc,
        fmt::format(fmt, fmt::make_format_args(std::forward<Args>(args)...)));
  }

  [[nodiscard]] Vec<Diagnosis>& diagnostics() noexcept { return m_diags; }
  [[nodiscard]] const Vec<Diagnosis>& diagnostics() const noexcept
  {
    return m_diags;
  }

  [[nodiscard]] bool has_errors() const noexcept
  {
    for (const auto& d : m_diags) {
      if (d.kind == Diagnosis::Kind::Error)
        return true;
    }

    return false;
  }

  [[nodiscard]] const String& path() const noexcept { return m_path; }
  [[nodiscard]] const String& source() const noexcept { return m_source; }

 private:
  // Helper to pretty-print a single diagnosis line with source context.
  void emit_one(const Diagnosis& d, spdlog::logger* logger) const;

 private:
  String m_path, m_name;
  const String& m_source;
  Vec<Diagnosis> m_diags{};
};

}  // namespace via

#endif
