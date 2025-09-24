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
#include <replxx.hxx>
#include "debug.h"
#include "ir/builder.h"
#include "ir/ir.h"
#include "manager.h"
#include "support/ansi.h"
#include "support/bit_enum.h"
#include "vm/machine.h"
#include "vm/value.h"

#ifdef VIA_PLATFORM_UNIX
    #include <dlfcn.h>
#elif defined(VIA_PLATFORM_WINDOWS)
    #include <windows.h>
#endif

// Modules are supposed to be allocated in a linear fashion, so we can get
// away with this
static via::ScopedAllocator module_allocator;

static via::Expected<std::string> read_file(const via::fs::path& path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        return via::Unexpected(
            std::format("No such file or directory: '{}'", path.string())
        );
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
        for (const HMODULE handle: libs) {
            FreeLibrary(handle);
        }
    }

    void push(const HMODULE handle) { libs.push_back(handle); }
} windowsLibs;
#endif

static via::Expected<via::NativeModuleInitCallback>
load_dylib_symbol(const via::fs::path& path, const char* symbol)
{
#ifdef VIA_PLATFORM_UNIX
    if (void* handle = dlopen(path.c_str(), RTLD_NOW)) {
        if (void* init = dlsym(handle, symbol)) {
            return reinterpret_cast<via::NativeModuleInitCallback>(init);
        }
    }

    return via::Unexpected(dlerror());
#else
    return via::Unexpected("Native modules not supported on host operating system");
#endif
}

via::Expected<via::Module*> via::Module::load_native_object(
    ModuleManager* manager,
    Module* importee,
    const char* name,
    const fs::path& path,
    const ast::StmtImport* ast_decl,
    const ModulePerms perms,
    const ModuleFlags flags
)
{
    if (manager->is_current_import(name)) {
        return Unexpected("Recursive import detected");
    }

    manager->push_import(name);

    if (manager->has_module(name)) {
        if (Module* module = manager->get_module(name); module->m_path == path) {
            manager->pop_import();
            return module;
        }
    }

    auto file = read_file(path);
    if (!file.has_value()) {
        manager->pop_import();
        return Unexpected(file.get_error());
    }

    auto* module = module_allocator.emplace<Module>();
    module->m_kind = ModuleKind::NATIVE;
    module->m_manager = manager;
    module->m_importee = importee;
    module->m_perms = perms;
    module->m_flags = flags;
    module->m_name = name;
    module->m_path = path;
    module->m_ast_decl = ast_decl;

    manager->push_module(module);

    auto symbol = std::format("{}{}", config::MODULE_ENTRY_PREFIX, name);
    auto callback = load_dylib_symbol(path, symbol.c_str());
    if (callback.has_error()) {
        return Unexpected(std::format(
            "Failed to load native module: {}",
            callback.get_error().to_string()
        ));
    }

    auto* module_info = (*callback)(manager);

    debug::require(module_info->begin != nullptr);

    for (size_t i = 0; i < module_info->size; i++) {
        const auto& entry = module_info->begin[i];
        module->m_defs[entry.id] = entry.def;
    }

    if (flags & ModuleFlags::DUMP_DEFTABLE) {
        std::println(
            std::cout,
            "{}",
            ansi::format(
                std::format("[deftable .{}]", name),
                ansi::Foreground::YELLOW,
                ansi::Background::BLACK,
                ansi::Style::BOLD
            )
        );

        for (const auto& def: module->m_defs) {
            std::println(std::cout, "  {}", def.second->to_string());
        }
    }

    manager->pop_import();
    return module;
}

via::Expected<via::Module*> via::Module::load_source_file(
    ModuleManager* manager,
    Module* importee,
    const char* name,
    const fs::path& path,
    const ast::StmtImport* ast_decl,
    const ModulePerms perms,
    const ModuleFlags flags
)
{
    if (manager->is_current_import(name)) {
        return Unexpected("Recursive import detected");
    }

    manager->push_import(name);

    if (manager->has_module(name)) {
        if (Module* module = manager->get_module(name); module->m_path == path) {
            manager->pop_import();
            return module;
        }
    }

    auto file = read_file(path);
    if (!file.has_value()) {
        manager->pop_import();
        return Unexpected(file.get_error());
    }

    auto* module = module_allocator.emplace<Module>();
    module->m_kind = ModuleKind::SOURCE;
    module->m_manager = manager;
    module->m_importee = importee;
    module->m_perms = perms;
    module->m_flags = flags;
    module->m_name = name;
    module->m_path = path;
    module->m_ast_decl = ast_decl;

    manager->push_module(module);

    DiagContext diags(path.string(), name, *file);

    Lexer lexer(*file);
    auto ttree = lexer.tokenize();

    Parser parser(*file, ttree, diags);
    auto ast = parser.parse();

    bool failed = diags.has_errors();
    if (failed)
        goto error;

    {
        IRBuilder ir_builder(module, ast, diags);
        module->m_ir = ir_builder.build();

        failed = diags.has_errors();
        if (failed)
            goto error;

        for (const auto& node: module->m_ir) {
            if (auto symbol = node->get_symbol()) {
                module->m_defs[*symbol] = Def::from(module->get_allocator(), node);
            }
        }

        Executable* exe = Executable::build_from_ir(module, module->m_ir);
        module->m_exe = exe;

        if ((flags & ModuleFlags::NO_EXECUTION) == 0) {
            VirtualMachine vm(module, exe);

            if (flags & ModuleFlags::DEBUG) {
                replxx::Replxx repl;

                std::println(std::cout, "Starting interactive VM debugger...");
                std::println(
                    std::cout,
                    "  > step      steps the interpreter\n"
                    "  > pc        dumps the interpreter program counter\n"
                    "  > regs      dumps the interpreter register buffer\n"
                    "  > stack     dumps the interpreter stack\n"
                );

                while (true) {
                    const char* cinput = repl.input("> ");
                    if (cinput == nullptr) {
                        break;
                    }

                    std::string input(cinput);
                    Snapshot snapshot(&vm);

                    if (input == "step") {
                        vm.execute_one();
                    }
                    else if (input == "pc") {
                        std::println(
                            std::cout,
                            "{}",
                            snapshot.program_counter.to_string()
                        );
                    }
                    else if (input == "regs") {
                        for (size_t index = 0; const auto& ptr: snapshot.registers) {
                            if (ptr != nullptr) {
                                std::println(
                                    std::cout,
                                    "R{} = {}",
                                    index,
                                    ptr->to_string()
                                );
                            }
                            index++;
                        }
                    }
                    else if (input == "stack") {
                        std::println(std::cout, "size: {}", snapshot.stack.size());

                        if (!snapshot.stack.empty()) {
                            const uintptr_t fp = snapshot.frame_ptr;
                            const uintptr_t* stk_base;

                            if (fp != 0) {
                                stk_base = snapshot.stack.begin().base() + fp;

                                auto old_fp = (uintptr_t*) *(stk_base - 0);
                                auto ret_pc = (Instruction*) *(stk_base - 1);
                                auto flags = (u64) * (stk_base - 2);
                                auto callee = (Value*) *(stk_base - 3);

                                std::cout << "Frame @ " << fp << "\n";
                                std::cout << "  callee   = " << callee->to_string()
                                          << "\n";
                                std::cout << "  flags    = " << flags << "\n";
                                std::cout << "  ret_pc   = " << (const void*) ret_pc
                                          << "\n";
                                std::cout << "  old_fp   = " << (const void*) old_fp
                                          << "\n";
                            }
                            else {
                                stk_base = snapshot.stack.begin().base() - 1;
                            }

                            for (auto* ptr = stk_base + 1;
                                 ptr < snapshot.stack.end().base() - 1;
                                 ++ptr) {
                                if (fp != 0)
                                    std::print(std::cout, "  ");

                                auto* val = (Value*) *ptr;
                                std::println(
                                    std::cout,
                                    "local {} = {}",
                                    ptr - stk_base - 1,
                                    val->to_string()
                                );
                            }
                        }
                    }
                    else {
                        std::println(std::cout, "{}", input);
                    }
                }
            }
            else {
                vm.execute();
            }
        }
    }

error:
    diags.emit();
    diags.clear();

    if (flags & ModuleFlags::DUMP_TTREE)
        std::println(std::cout, "{}", debug::to_string(ttree));
    if (flags & ModuleFlags::DUMP_AST)
        std::println(std::cout, "{}", debug::to_string(ast));
    if (flags & ModuleFlags::DUMP_IR)
        std::println(
            std::cout,
            "{}",
            debug::to_string(manager->get_symbol_table(), module->m_ir)
        );
    if (flags & ModuleFlags::DUMP_EXE)
        std::println(std::cout, "{}", module->m_exe->to_string());
    if (flags & ModuleFlags::DUMP_DEFTABLE) {
        std::println(
            std::cout,
            "{}",
            ansi::format(
                std::format("[deftable .{}]", name),
                ansi::Foreground::YELLOW,
                ansi::Background::BLACK,
                ansi::Style::BOLD
            )
        );

        for (const auto& def: module->m_defs) {
            std::println(std::cout, "  {}", def.second->to_string());
        }
    }

    if (failed) {
        for (Module* module = importee; module != nullptr; module = module->m_importee)
            spdlog::info(std::format("Imported by module '{}'", module->m_name));

        if ((flags & (ModuleFlags::DUMP_TTREE | ModuleFlags::DUMP_AST |
                      ModuleFlags::DUMP_IR) |
             ModuleFlags::DUMP_EXE) != 0u) {
            spdlog::warn("Dump may be invalid due to compilation failure");
        }
    }
    else {
        manager->pop_import();
        return module;
    }

    manager->pop_import();
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
    const via::ModuleManager& manager
)
{
    via::debug::require(!path.empty(), "bad import path");

    auto path_slice = path;
    auto& module_name = path_slice.back();
    path_slice.pop_back();

    // Lambda to try candidates in a given base path
    auto try_dir_candidates = [&](const via::fs::path& dir) -> via::Option<ModuleInfo> {
        via::fs::path path = dir;
        for (const auto& node: path_slice) {
            path /= node;
        }

        ModuleCandidate candidates[] = {
            {ModuleInfo::Kind::SOURCE, module_name + ".via"},
            {ModuleInfo::Kind::BINARY, module_name + ".viac"},
#ifdef VIA_PLATFORM_LINUX
            {ModuleInfo::Kind::NATIVE, module_name + ".so"},
#elifdef VIA_PLATFORM_WINDOWS
            {ModuleInfo::Kind::NATIVE, module_name + ".dll"},
#endif
        };

        auto try_path = [&](const via::fs::path& candidate,
                            ModuleInfo::Kind kind) -> via::Option<ModuleInfo> {
            if (via::fs::exists(candidate) && via::fs::is_regular_file(candidate))
                return ModuleInfo{.kind = kind, .path = candidate};
            else
                return via::nullopt;
        };

        for (const auto& c: candidates) {
            if (auto result = try_path(path / c.name, c.kind)) {
                return result;
            }
        }

        // Fallback: module in a subfolder "module.via"
        auto module_path = path / module_name / "module.via";
        if (auto result = try_path(module_path, ModuleInfo::Kind::SOURCE)) {
            return result;
        }

        return via::nullopt;
    };

    for (const auto& import_path: manager.get_import_paths()) {
        if (auto result = try_dir_candidates(import_path)) {
            return result;
        }
    }

    return via::nullopt;
}

via::Option<const via::Def*> via::Module::lookup(via::SymbolId symbol)
{
    if (auto it = m_defs.find(symbol); it != m_defs.end()) {
        return it->second;
    }
    return nullopt;
}

via::Expected<via::Module*>
via::Module::import(const QualName& path, const ast::StmtImport* ast_decl)
{
    debug::require(m_manager, "unmanaged module detected");

    auto module = resolveImportPath(m_path, path, *m_manager);
    if (!module.has_value()) {
        return Unexpected(std::format("Module '{}' not found", to_string(path)));
    }

    if ((m_perms & ModulePerms::IMPORT) == 0u) {
        return Unexpected("Current module lacks import capabilties");
    }

    switch (module->kind) {
    case ModuleInfo::Kind::SOURCE:
        return Module::load_source_file(
            m_manager,
            this,
            path.back().c_str(),
            module->path,
            ast_decl,
            m_perms,
            m_flags
        );
    case ModuleInfo::Kind::NATIVE:
        return Module::load_native_object(
            m_manager,
            this,
            path.back().c_str(),
            module->path,
            ast_decl,
            m_perms,
            m_flags
        );
    default:
        debug::todo("module types");
    }
}