// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "module.h"
#include "buffer.h"

#ifdef VIA_PLATFORM_UNIX
#include <dlfcn.h>
#elif defined(VIA_PLATFORM_WINDOWS)
#include <windows.h>
#endif

namespace via {

using ModuleInitFunc = const ModuleDef* (*)();

#ifdef VIA_PLATFORM_WINDOWS
static struct WinLibraryManager {
  Vec<HMODULE> libs;

  ~WinLibraryManager() {
    for (HMODULE handle : libs) {
      FreeLibrary(handle);
    }
  }
} win_libs;
#endif

static ModuleInitFunc load_symbol(const char* path, const char* symbol) {
#ifdef VIA_PLATFORM_UNIX
  if (void* handle = dlopen(path, RTLD_NOW)) {
    return (ModuleInitFunc)dlsym(handle, symbol);
  }
#elif defined(VIA_PLATFORM_WINDOWS)
  if (HMODULE handle = LoadLibraryA(path)) {
    win_libs.libs.push_back(handle);
    return (ModuleInitFunc)GetProcAddress(handle, symbol);
  }
#endif

  return NULL;
}

static String get_symbol(const char* name) {
  return fmt::format("{}{}", config::modules::init_prefix, name);
}

const ModuleDef* open_module(const char* path, const char* name) {
  auto symbol = get_symbol(name);
  auto init = load_symbol(path, symbol.c_str());
  return init != NULL ? init() : NULL;
}

}  // namespace via
