// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "module.h"

#ifdef VIA_PLATFORM_LINUX
#include <dlfcn.h>
#elif defined(VIA_PLATFORM_WINDOWS)
#include <windows.h>
#endif

namespace via {

constexpr const char* MODINIT_PREFIX = "viainit_";
constexpr const usize MODINIT_PREFIX_LEN = 8;

#ifdef VIA_PLATFORM_WINDOWS
static struct WinLibraryManager {
  Vec<HMODULE> libs;

  ~WinLibraryManager() {
    for (HMODULE handle : libs) {
      FreeLibrary(handle);
    }
  }
} _win_libs;
#endif

const ModuleDef* open_module(const char* path, const char* name) {
  // Compose symbol name like: viainit_<name>
  Buffer<char> symbuf{strlen(name) + MODINIT_PREFIX_LEN + 1};
  strcpy(symbuf.data, MODINIT_PREFIX);
  strcpy(symbuf.data + MODINIT_PREFIX_LEN, name);

#ifdef VIA_PLATFORM_UNIX
  void* handle = dlopen(path, RTLD_NOW);
  assert(handle);

  auto init = (ModuleInitFunc)dlsym(handle, symbuf.data);
  assert(init);

  return init();
#elif defined(VIA_PLATFORM_WINDOWS)
  HMODULE handle = LoadLibraryA(path);
  assert(handle);

  _win_libs.libs.push_back(handle);

  auto init = (ModuleInitFunc)GetProcAddress(handle, symbuf.data);
  assert(init);

  return init();
#endif
}

}  // namespace via
