/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "module.h"
#include <fstream>
#include <iostream>
#include "ansi.h"
#include "debug.h"
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
      std::format("No such file or directory: '{}'", path.string()));
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
  std::vector<HMODULE> libs;

  ~WinLibraryManager()
  {
    for (HMODULE handle : libs) {
      FreeLibrary(handle);
    }
  }

  void add(HMODULE handle) { libs.push_back(handle); }
} windowsLibs;
#endif

static via::Expected<via::NativeModuleInitCallback> osLoadSymbol(
  const via::fs::path& path,
  const char* symbol)
{
#ifdef VIA_PLATFORM_UNIX
  if (void* handle = dlopen(path.c_str(), RTLD_NOW)) {
    if (void* init = dlsym(handle, symbol)) {
      return reinterpret_cast<via::NativeModuleInitCallback>(init);
    }
  }

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
  const ast::StmtImport* decl,
  u32 perms,
  u32 flags)
{
  if (manager->isImporting(name)) {
    return Unexpected("Recursive import detected");
  }

  manager->pushImport(name);

  if (manager->hasModule(name)) {
    if (Module* module = manager->getModule(name); module->mPath == path) {
      manager->popImport();
      return module;
    }
  }

  auto file = readFile(path);
  if (!file.hasValue()) {
    manager->popImport();
    return Unexpected(file.getError());
  }

  {
    Module* module = stModuleAllocator.emplace<Module>();
    module->mKind = Kind::NATIVE;
    module->mManager = manager;
    module->mImportee = importee;
    module->mPerms = perms;
    module->mFlags = flags;
    module->mName = name;
    module->mPath = path;
    module->mDecl = decl;

    manager->addModule(module);

    auto symbol = std::format("{}{}", config::kInitCallbackPrefix, name);
    auto callback = osLoadSymbol(path, symbol.c_str());
    if (callback.hasError()) {
      return Unexpected(std::format("Failed to load native module: {}",
                                    callback.getError().toString()));
    }

    auto* moduleInfo = (*callback)(manager);

    debug::require(moduleInfo->begin != nullptr);

    for (usize i = 0; i < moduleInfo->size; i++) {
      const auto& entry = moduleInfo->begin[i];
      module->mDefs[entry.id] = entry.def;
    }

    if (flags & DUMP_DEFTABLE) {
      std::println(std::cout, "{}",
                   ansi::format(std::format("[deftable .{}]", name),
                                ansi::Foreground::Yellow,
                                ansi::Background::Black, ansi::Style::Bold));

      for (const auto& def : module->mDefs) {
        std::println(std::cout, "  {}", def.second->dump());
      }
    }

    manager->popImport();
    return module;
  }

  manager->popImport();
  return nullptr;
}

via::Expected<via::Module*> via::Module::loadSourceFile(
  ModuleManager* manager,
  Module* importee,
  const char* name,
  fs::path path,
  const ast::StmtImport* importDecl,
  u32 perms,
  u32 flags)
{
  if (manager->isImporting(name)) {
    return Unexpected("Recursive import detected");
  }

  manager->pushImport(name);

  if (manager->hasModule(name)) {
    if (Module* module = manager->getModule(name); module->mPath == path) {
      manager->popImport();
      return module;
    }
  }

  auto file = readFile(path);
  if (!file.hasValue()) {
    manager->popImport();
    return Unexpected(file.getError());
  }

  {
    Module* module = stModuleAllocator.emplace<Module>();
    module->mKind = Kind::SOURCE;
    module->mManager = manager;
    module->mImportee = importee;
    module->mPerms = perms;
    module->mFlags = flags;
    module->mName = name;
    module->mPath = path;
    module->mDecl = importDecl;

    manager->addModule(module);

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
      IRBuilder irb(module, ast, diags);
      module->mIr = irb.build();

      failed = diags.hasErrors();
      if (failed) {
        goto error;
      }

      for (const auto& node : module->mIr) {
        if (auto symbol = node->getSymbol()) {
          module->mDefs[*symbol] = Def::from(module->getAllocator(), node);
        }
      }

      {
        auto* exe = Executable::buildFromIR(module, module->mIr);
        module->mExecutable = exe;
      }
    }

  error:
    diags.emit();
    diags.clear();

    if (flags & DUMP_TTREE)
      std::println(std::cout, "{}", debug::dump(ttree));
    if (flags & DUMP_AST)
      std::println(std::cout, "{}", debug::dump(ast));
    if (flags & DUMP_IR)
      std::println(std::cout, "{}",
                   debug::dump(manager->getSymbolTable(), module->mIr));
    if (flags & DUMP_EXE)
      std::println(std::cout, "{}", module->mExecutable->dump());
    if (flags & DUMP_DEFTABLE) {
      std::println(std::cout, "{}",
                   ansi::format(std::format("[deftable .{}]", name),
                                ansi::Foreground::Yellow,
                                ansi::Background::Black, ansi::Style::Bold));

      for (const auto& def : module->mDefs) {
        std::println(std::cout, "  {}", def.second->dump());
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
      return module;
    }
  }

  manager->popImport();
  return nullptr;
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
  const via::fs::path& root,
  const via::QualName& path,
  const via::ModuleManager& manager)
{
  via::debug::require(!path.empty(), "bad import path");

  via::QualName pathSlice = path;
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

via::Option<const via::Def*> via::Module::lookup(via::SymbolId symbol)
{
  if (auto it = mDefs.find(symbol); it != mDefs.end()) {
    return it->second;
  }
  return nullopt;
}

via::Expected<via::Module*> via::Module::resolveImport(
  const QualName& path,
  const ast::StmtImport* importDecl)
{
  debug::require(mManager, "unmanaged module detected");

  auto module = resolveImportPath(mPath, path, *mManager);
  if (!module.hasValue()) {
    return Unexpected(std::format("Module '{}' not found", toString(path)));
  }

  if ((mPerms & Perms::IMPORT) == 0u) {
    return Unexpected("Current module lacks import capabilties");
  }

  switch (module->kind) {
    case ModuleInfo::Kind::SOURCE:
      return Module::loadSourceFile(mManager, this, path.back().c_str(),
                                    module->path, importDecl, mPerms, mFlags);
    case ModuleInfo::Kind::NATIVE:
      return Module::loadNativeObject(mManager, this, path.back().c_str(),
                                      module->path, importDecl, mPerms, mFlags);
    default:
      debug::todo("module types");
  }
}
