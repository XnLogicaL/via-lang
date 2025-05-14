// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "context.h"

namespace via {

// ==========================================================================================
// Context
Context::Context(const std::string& file_path, const std::string& file_source)
  : file_path(file_path),
    file_source(file_source) {}

Context::Context(const byte_stream_t&) {}

void Context::clear() {}

const char* Context::get_platform_info() {
  static char buffer[32];
  static bool fetched = false;

#ifdef _WIN32
  const char* os = "windows";
#elifdef __linux__
  const char* os = "linux";
#else
  const char* os = "other";
#endif

#ifdef __x86_64__
  const char* arch = "x86-64";
#elifdef i386
  const char* arch = "x86-32";
#elifdef __aarch64__
  const char* arch = "arm-64";
#else
  const char* arch = "other";
#endif

  if (!fetched) {
    fetched = true;
    std::snprintf(buffer, sizeof(buffer), "%s-%s", os, arch);
  }

  return buffer;
}

byte_stream_t Context::encode() {
  return {};
}

// ==========================================================================================
// CompilerContext

} // namespace via
