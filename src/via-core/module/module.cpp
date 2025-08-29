// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "module.h"
#include <fstream>
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

static Allocator module_alloc;

static Result<String, String> read_file(const fs::path& path)
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

static String get_symbol(const char* name)
{
  return fmt::format("viainit_{}", name);
}

}  // namespace native

Result<Module*, String> Module::from_source(ModuleManager* mm,
                                            Module* importee,
                                            const char* name,
                                            fs::path path,
                                            u32 perms,
                                            u32 flags)
{
  if (mm->is_importing(name)) {
    return std::unexpected("Recursive import detected");
  }

  mm->push_import(name);

  if (mm->has_module(name)) {
    if (Module* m = mm->get_module(name); m->m_path == path) {
      mm->pop_import();
      return m;
    }
  }

  auto file = read_file(path);
  if (!file.has_value()) {
    mm->pop_import();
    return std::unexpected(file.error());
  }

  {
    Module* m = module_alloc.emplace<Module>();
    m->m_manager = mm;
    m->m_importee = importee;
    m->m_perms = perms;
    m->m_flags = flags;
    m->m_name = name;
    m->m_path = path;

    mm->add_module(m);

    DiagContext diags(path.string(), name, *file);

    Lexer lexer(*file);
    auto tt = lexer.tokenize();

    Parser parser(*file, tt, diags);
    auto ast = parser.parse();

    bool failed = diags.has_errors();
    if (failed) {
      goto error;
    }

    {
      // reset semantic state
      sema::registers::reset();
      sema::stack::reset();

      IRBuilder irb(m, ast, diags);
      m->m_ir = irb.build();

      failed = diags.has_errors();
      if (failed) {
        goto error;
      }

      for (const ir::Entity* e : m->m_ir) {
        m->m_defs[e->symbol] = Def::from(m, e);
      }
    }

  error:
    diags.emit();
    diags.clear();

    if (flags & DUMP_TTREE)
      debug::dump(tt);
    if (flags & DUMP_AST)
      debug::dump(ast);

    if (failed) {
      for (Module* m = importee; m != nullptr; m = m->m_importee)
        spdlog::info("Imported by module '{}'", m->m_name);

      if ((flags & (DUMP_TTREE | DUMP_AST | DUMP_IR)) != 0u) {
        spdlog::warn("Dump may be invalid due to compilation failure");
      }
    } else {
      mm->pop_import();
      return m;
    }
  }

  mm->pop_import();
  return nullptr;
}

[[nodiscard]] Optional<SymbolInfo> Module::lookup(const QualPath& qs)
{
  if (m_manager == nullptr) {
    return nullopt;
  }

  const auto& modules = m_manager->get_modules();
  if (auto it = modules.find(qs[0]); it != modules.end()) {
    Module* module = it->second;
    QualPath new_qs = qs;
    new_qs.pop_front();

    SymbolId id = module->m_symbols.intern(new_qs);

    return SymbolInfo{
        .def = module->m_defs[id],
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

static Optional<ModuleInfo> resolve_import_path(fs::path root,
                                                const QualPath& qs)
{
  assert(!qs.empty());

  QualPath slice = qs;
  String module_name = slice.back();
  slice.pop_back();

  fs::path path;

  if (qs[0] == "std") {
#ifdef VIA_PLATFORM_WINDOWS
    const char* default_path = "C:\\Program Files\\via\\std\\";
#elifdef VIA_PLATFORM_UNIX
    const char* default_path = "/usr/lib/via/std/";
#endif
    const char* env_path = std::getenv("VIA_PATH");
    path = env_path ? env_path : default_path;
  } else {
    path = root.parent_path();
  }

  for (const auto& node : slice) {
    path /= node;
  }

  ModuleCandidate candidates[] = {
      {ModuleInfo::Kind::SOURCE, module_name + ".via"},
      {ModuleInfo::Kind::BINARY, module_name + ".viac"},
#ifdef VIA_PLATFORM_WINDOWS
      {ModuleInfo::Kind::NATIVE, module_name + ".dll"},
#elif defined(VIA_PLATFORM_UNIX)
      {ModuleInfo::Kind::NATIVE,
       "lib" + module_name + ".so"},  // Linux/BSD style
#elif defined(VIA_PLATFORM_MACOS)
      {ModuleInfo::Kind::NATIVE, "lib" + module_name + ".dylib"},
#endif
  };

  auto try_path = [&](const fs::path& candidate,
                      ModuleInfo::Kind kind) -> Optional<ModuleInfo> {
    if (fs::exists(candidate) && fs::is_regular_file(candidate)) {
      return ModuleInfo{.kind = kind, .path = candidate};
    } else {
      return nullopt;
    }
  };

  for (const auto& c : candidates) {
    auto result = try_path(path / c.filename, c.kind);
    if (result)
      return result;
  }

  auto module_path = path / module_name / "module.via";
  if (auto result = try_path(module_path, ModuleInfo::Kind::SOURCE)) {
    return result;
  }

  return nullopt;
}

Result<Module*, String> Module::resolve_import(const QualPath& qs)
{
  auto module = resolve_import_path(m_path, qs);
  if (!module.has_value()) {
    return std::unexpected(fmt::format("Module '{}' not found", to_string(qs)));
  }

  if ((m_perms & Perms::IMPORT) == 0u) {
    return std::unexpected("Current module lacks import capabilties");
  }

  switch (module->kind) {
    case ModuleInfo::Kind::SOURCE:
      return Module::from_source(m_manager, this, qs.back().c_str(),
                                 module->path, m_perms, 0);
    default:
      debug::todo("module types");
  }
}

}  // namespace via
