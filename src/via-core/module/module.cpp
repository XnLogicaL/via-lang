// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "module.h"
#include <fstream>
#include "color.h"
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

#ifdef VIA_PLATFORM_WINDOWS
static struct WinLibraryManager
{
  Vec<HMODULE> libs;

  ~WinLibraryManager()
  {
    for (HMODULE handle : libs) {
      FreeLibrary(handle);
    }
  }

  void add(HMODULE handle) { libs.push_back(handle); }
} windowsLibs;
#endif

static Result<Module::InitCallback, String> loadSymbol(const fs::path& path,
                                                       const char* symbol)
{
#ifdef VIA_PLATFORM_UNIX
  void* handle = dlopen(path.c_str(), RTLD_NOW);
  if (handle == nullptr) {
    goto error;
  }

  {
    void* sym = dlsym(handle, symbol);
    if (sym == nullptr) {
      goto error;
    }

    return reinterpret_cast<Module::InitCallback>(sym);
  }

error:
  return std::unexpected(dlerror());
#elifdef VIA_PLATFORM_WINDOWS
  // LoadLibrary accepts LPCWSTR or LPCSTR depending on unicode macros; we
  // assume ANSI path.
  HMODULE handle = LoadLibraryA(path.string().c_str());
  if (!handle)
    return nullptr;
  windowsLibs.add(handle);
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

Result<Module*, String> Module::loadNativeObject(ModuleManager* mgr,
                                                 Module* importee,
                                                 const char* name,
                                                 fs::path path,
                                                 u32 perms,
                                                 u32 flags)
{
  if (mgr->isImporting(name)) {
    return std::unexpected("Recursive import detected");
  }

  mgr->pushImport(name);

  if (mgr->hasModule(name)) {
    if (Module* m = mgr->getModule(name); m->mPath == path) {
      mgr->popImport();
      return m;
    }
  }

  auto file = readFile(path);
  if (!file.has_value()) {
    mgr->popImport();
    return std::unexpected(file.error());
  }

  {
    Module* m = moduleAlloc.emplace<Module>();
    m->mKind = Kind::NATIVE;
    m->mManager = mgr;
    m->mImportee = importee;
    m->mPerms = perms;
    m->mFlags = flags;
    m->mName = name;
    m->mPath = path;

    mgr->addModule(m);

    auto symbol = native::getSymbol(name);
    auto callback = native::loadSymbol(path, symbol.c_str());
    if (!callback) {
      return std::unexpected(
          fmt::format("Failed to load native module: {}", callback.error()));
    }

    auto* moduleInfo = (*callback)(mgr);

    debug::assertm(moduleInfo->dt != nullptr);

    for (usize i = 0; i < moduleInfo->size; i++) {
      const DefTableEntry& dte = moduleInfo->dt[i];
      m->mDefs[dte.id] = dte.def;
    }

    if (flags & DUMP_DEFTABLE) {
      fmt::println("{}", ansiFormat(fmt::format("[deftable .{}]", name),
                                    Fg::Yellow, Bg::Black, Style::Bold));

      for (const auto& def : m->mDefs) {
        fmt::println("  {}", def.second->dump());
      }
    }

    mgr->popImport();
    return m;
  }

  mgr->popImport();
  return nullptr;
}

Result<Module*, String> Module::loadSourceFile(ModuleManager* mgr,
                                               Module* importee,
                                               const char* name,
                                               fs::path path,
                                               u32 perms,
                                               u32 flags)
{
  if (mgr->isImporting(name)) {
    return std::unexpected("Recursive import detected");
  }

  mgr->pushImport(name);

  if (mgr->hasModule(name)) {
    if (Module* m = mgr->getModule(name); m->mPath == path) {
      mgr->popImport();
      return m;
    }
  }

  auto file = readFile(path);
  if (!file.has_value()) {
    mgr->popImport();
    return std::unexpected(file.error());
  }

  {
    Module* m = moduleAlloc.emplace<Module>();
    m->mKind = Kind::SOURCE;
    m->mManager = mgr;
    m->mImportee = importee;
    m->mPerms = perms;
    m->mFlags = flags;
    m->mName = name;
    m->mPath = path;

    mgr->addModule(m);

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
        m->mDefs[e->symbol] = Def::from(m->getAllocator(), e);
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
    if (flags & DUMP_DEFTABLE) {
      fmt::println("{}", ansiFormat(fmt::format("[deftable .{}]", name),
                                    Fg::Yellow, Bg::Black, Style::Bold));

      for (const auto& def : m->mDefs) {
        fmt::println("  {}", def.second->dump());
      }
    }

    if (failed) {
      for (Module* m = importee; m != nullptr; m = m->mImportee)
        spdlog::info("Imported by module '{}'", m->mName);

      if ((flags & (DUMP_TTREE | DUMP_AST | DUMP_IR)) != 0u) {
        spdlog::warn("Dump may be invalid due to compilation failure");
      }
    } else {
      mgr->popImport();
      return m;
    }
  }

  mgr->popImport();
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

    SymbolId id = module->mManager->mSymbols.intern(new_qs);

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

static Optional<ModuleInfo> resolveImportPath(fs::path root,
                                              const QualPath& qs,
                                              const ModuleManager& mgr)
{
  debug::assertm(!qs.empty(), "bad import path");

  QualPath slice = qs;
  String moduleName = slice.back();
  slice.pop_back();

  // Lambda to try candidates in a given base path
  auto tryCandidatesInPath =
      [&](const fs::path& basePath) -> Optional<ModuleInfo> {
    fs::path path = basePath;
    for (const auto& node : slice) {
      path /= node;
    }

    ModuleCandidate candidates[] = {
        {ModuleInfo::Kind::SOURCE, moduleName + ".via"},
        {ModuleInfo::Kind::BINARY, moduleName + ".viac"},
#ifdef VIA_PLATFORM_LINUX
        {ModuleInfo::Kind::NATIVE, moduleName + ".so"},
#elifdef VIA_PLATFORM_WINDOWS
        {ModuleInfo::Kind::NATIVE, moduleName + ".dll"},
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
      if (auto result = tryPath(path / c.filename, c.kind)) {
        return result;
      }
    }

    // Fallback: module in a subfolder "module.via"
    auto modulePath = path / moduleName / "module.via";
    if (auto result = tryPath(modulePath, ModuleInfo::Kind::SOURCE)) {
      return result;
    }

    return nullopt;
  };

  for (const auto& importPath : mgr.getImportPaths()) {
    if (auto result = tryCandidatesInPath(importPath)) {
      return result;
    }
  }

  return nullopt;
}

Result<Module*, String> Module::resolveImport(const QualPath& qs)
{
  debug::assertm(mManager, "unmanaged module detected");

  auto module = resolveImportPath(mPath, qs, *mManager);
  if (!module.has_value()) {
    return std::unexpected(fmt::format("Module '{}' not found", toString(qs)));
  }

  if ((mPerms & Perms::IMPORT) == 0u) {
    return std::unexpected("Current module lacks import capabilties");
  }

  switch (module->kind) {
    case ModuleInfo::Kind::SOURCE:
      return Module::loadSourceFile(mManager, this, qs.back().c_str(),
                                    module->path, mPerms, mFlags);
    case ModuleInfo::Kind::NATIVE:
      return Module::loadNativeObject(mManager, this, qs.back().c_str(),
                                      module->path, mPerms, mFlags);
    default:
      debug::todo("module types");
  }
}

}  // namespace via
