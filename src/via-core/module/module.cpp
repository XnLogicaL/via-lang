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

// Modules are supposed to be allocated in a linear fashion, so we can get
// away with this
static via::Allocator stModuleAllocator;

static via::Expected<std::string> readFile(const via::fs::path& path)
{
  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    return via::Unexpected(
      fmt::format("No such file or directory: '{}'", path.string()));
  }

  std::ostringstream oss;
  std::string line;

  while (std::getline(ifs, line)) {
    oss << line << '\n';
  }

  return oss.str();
}

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

static via::Expected<via::NativeModuleInitCallback> loadSymbol(
  const via::fs::path& path,
  const char* symbol)
{
#ifdef VIA_PLATFORM_UNIX
  void* handle = dlopen(path.c_str(), RTLD_NOW);
  if (handle == nullptr) {
    goto error;
  }

  {
    void* func = dlsym(handle, symbol);
    if (func == nullptr) {
      goto error;
    }

    return reinterpret_cast<via::NativeModuleInitCallback>(func);
  }

error:
  return via::Unexpected(dlerror());
#else
  return via::Unexpected(
    "Native modules not supported on host operating system");
#endif
}

via::Expected<via::Module*> via::Module::loadNativeObject(
  ModuleManager* manager,
  Module* importee,
  const char* name,
  fs::path path,
  u32 perms,
  u32 flags)
{
  if (manager->isImporting(name)) {
    return Unexpected("Recursive import detected");
  }

  manager->pushImport(name);

  if (manager->hasModule(name)) {
    if (Module* m = manager->getModule(name); m->mPath == path) {
      manager->popImport();
      return m;
    }
  }

  auto file = readFile(path);
  if (!file.hasValue()) {
    manager->popImport();
    return Unexpected(file.getError());
  }

  {
    Module* m = stModuleAllocator.emplace<Module>();
    m->mKind = Kind::NATIVE;
    m->mManager = manager;
    m->mImportee = importee;
    m->mPerms = perms;
    m->mFlags = flags;
    m->mName = name;
    m->mPath = path;

    manager->addModule(m);

    auto symbol = fmt::format("{}{}", config::kInitCallbackPrefix, name);
    auto callback = loadSymbol(path, symbol.c_str());
    if (callback.hasError()) {
      return Unexpected(fmt::format("Failed to load native module: {}",
                                    callback.getError().toString()));
    }

    auto* moduleInfo = (*callback)(manager);

    debug::assertm(moduleInfo->begin != nullptr);

    for (usize i = 0; i < moduleInfo->size; i++) {
      const auto& entry = moduleInfo->begin[i];
      m->mDefs[entry.id] = entry.def;
    }

    if (flags & DUMP_DEFTABLE) {
      fmt::println("{}", ansiFormat(fmt::format("[deftable .{}]", name),
                                    Fg::Yellow, Bg::Black, Style::Bold));

      for (const auto& def : m->mDefs) {
        fmt::println("  {}", def.second->dump());
      }
    }

    manager->popImport();
    return m;
  }

  manager->popImport();
  return nullptr;
}

via::Expected<via::Module*> via::Module::loadSourceFile(ModuleManager* manager,
                                                        Module* importee,
                                                        const char* name,
                                                        fs::path path,
                                                        u32 perms,
                                                        u32 flags)
{
  if (manager->isImporting(name)) {
    return Unexpected("Recursive import detected");
  }

  manager->pushImport(name);

  if (manager->hasModule(name)) {
    if (Module* m = manager->getModule(name); m->mPath == path) {
      manager->popImport();
      return m;
    }
  }

  auto file = readFile(path);
  if (!file.hasValue()) {
    manager->popImport();
    return Unexpected(file.takeError());
  }

  {
    Module* m = stModuleAllocator.emplace<Module>();
    m->mKind = Kind::SOURCE;
    m->mManager = manager;
    m->mImportee = importee;
    m->mPerms = perms;
    m->mFlags = flags;
    m->mName = name;
    m->mPath = path;

    manager->addModule(m);

    DiagContext diags(path.string(), name, *file);

    Lexer lexer(*file);
    auto ttree = lexer.tokenize();

    Parser parser(*file, ttree, diags);
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

      for (const auto& node : m->mIr) {
        if (auto symbol = node->getSymbol()) {
          m->mDefs[*symbol] = Def::from(m->getAllocator(), node);
        }
      }
    }

  error:
    diags.emit();
    diags.clear();

    if (flags & DUMP_TTREE)
      fmt::println("{}", debug::dump(ttree));
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
      for (Module* module = importee; module != nullptr;
           module = module->mImportee)
        spdlog::info("Imported by module '{}'", module->mName);

      if ((flags & (DUMP_TTREE | DUMP_AST | DUMP_IR)) != 0u) {
        spdlog::warn("Dump may be invalid due to compilation failure");
      }
    } else {
      manager->popImport();
      return m;
    }
  }

  manager->popImport();
  return nullptr;
}

[[nodiscard]] via::Option<via::SymbolInfo> via::Module::lookup(
  const QualPath& path)
{
  if (mManager == nullptr) {
    return via::nullopt;
  }

  const auto& modules = mManager->getModules();
  if (auto it = modules.find(path[0]); it != modules.end()) {
    Module* module = it->second;
    QualPath new_qs = path;
    new_qs.pop_front();

    SymbolId id = SymbolTable::getInstance().intern(new_qs);

    return SymbolInfo{
      .symbol = module->mDefs[id],
      .module = module,
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

  via::fs::path path;
};

struct ModuleCandidate
{
  ModuleInfo::Kind kind;
  std::string name;
};

static via::Option<ModuleInfo> resolveImportPath(
  via::fs::path root,
  const via::QualPath& path,
  const via::ModuleManager& manager)
{
  via::debug::assertm(!path.empty(), "bad import path");

  via::QualPath pathSlice = path;
  auto& moduleName = pathSlice.back();
  pathSlice.pop_back();

  // Lambda to try candidates in a given base path
  auto tryCandidatesInPath =
    [&](const via::fs::path& basePath) -> via::Option<ModuleInfo> {
    via::fs::path path = basePath;
    for (const auto& node : pathSlice) {
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

    auto tryPath = [&](const via::fs::path& candidate,
                       ModuleInfo::Kind kind) -> via::Option<ModuleInfo> {
      if (via::fs::exists(candidate) && via::fs::is_regular_file(candidate)) {
        return ModuleInfo{.kind = kind, .path = candidate};
      } else {
        return via::nullopt;
      }
    };

    for (const auto& c : candidates) {
      if (auto result = tryPath(path / c.name, c.kind)) {
        return result;
      }
    }

    // Fallback: module in a subfolder "module.via"
    auto modulePath = path / moduleName / "module.via";
    if (auto result = tryPath(modulePath, ModuleInfo::Kind::SOURCE)) {
      return result;
    }

    return via::nullopt;
  };

  for (const auto& importPath : manager.getImportPaths()) {
    if (auto result = tryCandidatesInPath(importPath)) {
      return result;
    }
  }

  return via::nullopt;
}

via::Expected<via::Module*> via::Module::resolveImport(const QualPath& path)
{
  debug::assertm(mManager, "unmanaged module detected");

  auto module = resolveImportPath(mPath, path, *mManager);
  if (!module.hasValue()) {
    return Unexpected(fmt::format("Module '{}' not found", toString(path)));
  }

  if ((mPerms & Perms::IMPORT) == 0u) {
    return Unexpected("Current module lacks import capabilties");
  }

  switch (module->kind) {
    case ModuleInfo::Kind::SOURCE:
      return Module::loadSourceFile(mManager, this, path.back().c_str(),
                                    module->path, mPerms, mFlags);
    case ModuleInfo::Kind::NATIVE:
      return Module::loadNativeObject(mManager, this, path.back().c_str(),
                                      module->path, mPerms, mFlags);
    default:
      debug::todo("module types");
  }
}
