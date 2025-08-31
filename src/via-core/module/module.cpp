// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "module.h"
#include <fstream>
#include "debug.h"
#include "ir/builder.h"
#include "manager.h"
#include "sema/register.h"
#include "sema/stack.h"

#ifdef VIA_PLATFORM_UNIX
  #include <dlfcn.h>
#elif defined(VIA_PLATFORM_WINDOWS)
  #include <windows.h>
#endif

namespace via
{

static Allocator moduleAlloc;

static Result<String, String> readFile(const fs::path& path)
{
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    return std::unexpected(
        fmt::format("No such file or directory: '{}'", path.string()));
  }

  String line;
  String content;
  while (std::getline(ifs, line)) {
    content += line;
    content += '\n';
  }

  return content;
}

namespace native
{

using ModuleInitFunc = const Module* (*)(ModuleManager* mm);

#ifdef VIA_PLATFORM_WINDOWS
struct WinLibraryManager
{
  Vec<HMODULE> libs;

  ~WinLibraryManager()
  {
    for (HMODULE handle : libs) {
      FreeLibrary(handle);
    }
  }

  void add(HMODULE h) { libs.push_back(h); }
};

static WinLibraryManager win_libs;
#endif

static ModuleInitFunc load_symbol(const fs::path& path, const char* symbol)
{
#ifdef VIA_PLATFORM_UNIX
  void* handle = dlopen(path.c_str(), RTLD_NOW);
  if (!handle)
    return nullptr;
  void* sym = dlsym(handle, symbol);
  return reinterpret_cast<ModuleInitFunc>(sym);
#elif defined(VIA_PLATFORM_WINDOWS)
  // LoadLibrary accepts LPCWSTR or LPCSTR depending on unicode macros; we
  // assume ANSI path.
  HMODULE handle = LoadLibraryA(path.string().c_str());
  if (!handle)
    return nullptr;
  win_libs.add(handle);
  FARPROC proc = GetProcAddress(handle, symbol);
  return reinterpret_cast<ModuleInitFunc>(proc);
#else
  (void)path;
  (void)symbol;
  return nullptr;
#endif
}

static String getSymbol(const char* name)
{
  return fmt::format("viainit_{}", name);
}

}  // namespace native

Result<Module*, String> Module::fromSource(ModuleManager* mm,
                                           Module* importee,
                                           const char* name,
                                           fs::path path,
                                           u32 perms,
                                           u32 flags)
{
  if (mm->isImporting(name)) {
    return std::unexpected("Recursive import detected");
  }

  mm->pushImport(name);

  if (mm->hasModule(name)) {
    if (Module* m = mm->getModule(name); m->mPath == path) {
      mm->popImport();
      return m;
    }
  }

  auto file = readFile(path);
  if (!file.has_value()) {
    mm->popImport();
    return std::unexpected(file.error());
  }

  {
    Module* m = moduleAlloc.emplace<Module>();
    m->mManager = mm;
    m->mImportee = importee;
    m->mPerms = perms;
    m->mFlags = flags;
    m->mName = name;
    m->mPath = path;

    mm->addModule(m);

    DiagContext diags(path.string(), name, *file);

    Lexer lexer(*file);
    auto tt = lexer.tokenize();

    Parser parser(*file, tt, diags);
    auto ast = parser.parse();

    bool failed = diags.hasErrors();
    if (failed) {
      goto error;
    }

    {
      // reset semantic state
      sema::registers::reset();
      sema::stack::reset();

      ir::Builder irb(m, ast, diags);
      m->mIr = irb.build();

      failed = diags.hasErrors();
      if (failed) {
        goto error;
      }

      for (const ir::Entity* e : m->mIr) {
        m->mDefs[e->symbol] = Def::from(m, e);
      }
    }

  error:
    diags.emit();
    diags.clear();

    if (flags & DUMP_TTREE)
      fmt::println("{}", debug::dump(tt));
    if (flags & DUMP_AST)
      fmt::println("{}", debug::dump(ast));
    if (flags & DUMP_IR)
      fmt::println("{}", debug::dump(m->mIr));

    if (failed) {
      for (Module* m = importee; m != nullptr; m = m->mImportee)
        spdlog::info("Imported by module '{}'", m->mName);

      if ((flags & (DUMP_TTREE | DUMP_AST | DUMP_IR)) != 0u) {
        spdlog::warn("Dump may be invalid due to compilation failure");
      }
    } else {
      mm->popImport();
      return m;
    }
  }

  mm->popImport();
  return nullptr;
}

[[nodiscard]] Optional<SymbolInfo> Module::lookup(const QualPath& qs)
{
  if (mManager == nullptr) {
    return nullopt;
  }

  const auto& modules = mManager->getModules();
  if (auto it = modules.find(qs[0]); it != modules.end()) {
    Module* module = it->second;
    QualPath new_qs = qs;
    new_qs.pop_front();

    SymbolId id = module->mSymbols.intern(new_qs);

    return SymbolInfo{
        .def = module->mDefs[id],
        .mod = module,
    };
  } else {
    return nullopt;
  }
}

struct ModuleInfo
{
  enum class Kind
  {
    SOURCE,
    BINARY,
    NATIVE,
  } kind;

  fs::path path;
};

struct ModuleCandidate
{
  ModuleInfo::Kind kind;
  String filename;
};

static Optional<ModuleInfo> resolveImportPath(fs::path root, const QualPath& qs)
{
  debug::assertm(!qs.empty(), "bad import path");

  QualPath slice = qs;
  String moduleName = slice.back();
  slice.pop_back();

  fs::path path;

  if (qs[0] == "std") {
#ifdef VIA_PLATFORM_WINDOWS
    const char* defaultPath = "C:\\Program Files\\via\\std\\";
#elifdef VIA_PLATFORM_UNIX
    const char* defaultPath = "/usr/lib/via/std/";
#endif
    const char* envPath = std::getenv("VIA_PATH");
    path = envPath ? envPath : defaultPath;
  } else {
    path = root.parent_path();
  }

  for (const auto& node : slice) {
    path /= node;
  }

  ModuleCandidate candidates[] = {
      {ModuleInfo::Kind::SOURCE, moduleName + ".via"},
      {ModuleInfo::Kind::BINARY, moduleName + ".viac"},
#ifdef VIA_PLATFORM_WINDOWS
      {ModuleInfo::Kind::NATIVE, moduleName + ".dll"},
#elif defined(VIA_PLATFORM_UNIX)
      {ModuleInfo::Kind::NATIVE,
       "lib" + moduleName + ".so"},  // Linux/BSD style
#elif defined(VIA_PLATFORM_MACOS)
      {ModuleInfo::Kind::NATIVE, "lib" + moduleName + ".dylib"},
#endif
  };

  auto tryPath = [&](const fs::path& candidate,
                     ModuleInfo::Kind kind) -> Optional<ModuleInfo> {
    if (fs::exists(candidate) && fs::is_regular_file(candidate)) {
      return ModuleInfo{.kind = kind, .path = candidate};
    } else {
      return nullopt;
    }
  };

  for (const auto& c : candidates) {
    auto result = tryPath(path / c.filename, c.kind);
    if (result) {
      return result;
    }
  }

  auto modulePath = path / moduleName / "module.via";
  if (auto result = tryPath(modulePath, ModuleInfo::Kind::SOURCE)) {
    return result;
  }

  return nullopt;
}

Result<Module*, String> Module::resolveImport(const QualPath& qs)
{
  auto module = resolveImportPath(mPath, qs);
  if (!module.has_value()) {
    return std::unexpected(fmt::format("Module '{}' not found", toString(qs)));
  }

  if ((mPerms & Perms::IMPORT) == 0u) {
    return std::unexpected("Current module lacks import capabilties");
  }

  switch (module->kind) {
    case ModuleInfo::Kind::SOURCE:
      return Module::fromSource(mManager, this, qs.back().c_str(), module->path,
                                mPerms, 0);
    default:
      debug::todo("module types");
  }
}

}  // namespace via
