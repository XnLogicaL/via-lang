/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "module.hpp"
#include <expected>
#include <format>
#include <fstream>
#include <iostream>
#include <replxx.hxx>
#include "debug.hpp"
#include "ir/builder.hpp"
#include "manager.hpp"
#include "source.hpp"
#include "support/memory.hpp"
#include "support/os/dl.hpp"
#include "symbol.hpp"
#include "vm/debugger.hpp"
#include "vm/machine.hpp"

// Read a file into a string
// clang-format off
static std::expected<std::string, std::string>
read_file(const std::filesystem::path& path)
{
    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        return std::unexpected(
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
// clang-format on

// Load a shared library as a native module object
std::expected<via::Module*, std::string> via::Module::load_native_object(
    ModuleManager& manager,
    Module* importee,
    const char* name,
    const std::filesystem::path& path,
    const ast::StmtImport* ast_decl,
    const ModulePerms perms,
    const ModuleFlags flags
)
{
    // Check if the module is being recursively imported
    if (manager.is_current_import(name)) {
        return std::unexpected("Recursive import detected");
    }

    // Push the module as an import
    manager.push_import(name);

    // Check if the module is already loaded
    if (manager.has_module(name)) {
        // Check if the cached module is the same as the one being imported
        if (Module* module = manager.get_module(name); module->m_path == path) {
            manager.pop_import(); // Pop the import stack
            return module;        // Return the cached module
        }
    }

    auto& alloc = manager.allocator();
    auto dylib = os::DynamicLibrary::load_library(path);
    if (!dylib.has_value()) {
        return std::unexpected(dylib.error());
    }

    // Instantiate the module
    auto* module = alloc.emplace<Module>(manager, SourceBuffer{});
    module->m_kind = ModuleKind::NATIVE;
    module->m_importee = importee;
    module->m_perms = perms;
    module->m_flags = flags;
    module->m_name = name;
    module->m_path = path;
    module->m_ast_decl = ast_decl;

    // Register the module with the manager
    manager.push_module(module);

    // Find the module's entry point
    auto symbol = std::format("{}{}", config::MODULE_ENTRY_PREFIX, name);
    auto callback = dylib->load_symbol<NativeModuleInitCallback>(symbol.c_str());
    if (!callback.has_value()) {
        return std::unexpected(
            std::format("Failed to load native module: {}", callback.error())
        );
    }

    module->m_dl = std::move(*dylib);

    // Retrieve module information
    auto* module_info = (*callback)(&manager);

    // Validate module information
    debug::require(module_info->begin != nullptr);

    // Map module definitions
    for (size_t i = 0; i < module_info->size; i++) {
        const auto& entry = module_info->begin[i];
        module->m_defs[entry.id] = entry.def;
    }

    if (flags & ModuleFlags::DUMP_DEFTABLE)
        std::cout << std::format("({}) ", module->m_name)
                  << to_string(module->m_manager.symbol_table(), module->m_defs);

    // Pop import stack
    manager.pop_import();
    return module;
}

// Load source file as a module
std::expected<via::Module*, std::string> via::Module::load_source_file(
    ModuleManager& manager,
    Module* importee,
    const char* name,
    const std::filesystem::path& path,
    const ast::StmtImport* ast_decl,
    const ModulePerms perms,
    const ModuleFlags flags
)
{
    // Check if the module is being recursively imported
    if (manager.is_current_import(name)) {
        return std::unexpected("Recursive import detected");
    }

    // Push import stack
    manager.push_import(name);

    // Check if the module is already loaded
    if (manager.has_module(name)) {
        // Check if the loaded module is the same as the one being imported
        if (Module* module = manager.get_module(name); module->m_path == path) {
            manager.pop_import(); // Pop the import stack
            return module;        // Return the cached module
        }
    }

    // Read the source file
    auto file = read_file(path);
    if (!file.has_value()) {
        manager.pop_import();
        return std::unexpected(file.error());
    }

    // Instantiate the module
    auto* module = manager.allocator().emplace<Module>(manager, std::move(*file));
    module->m_kind = ModuleKind::SOURCE;
    module->m_importee = importee;
    module->m_perms = perms;
    module->m_flags = flags;
    module->m_name = name;
    module->m_path = path;
    module->m_ast_decl = ast_decl;

    // Register the module with the manager
    manager.push_module(module);

    // Instantiate diagnostics context
    DiagContext diags(path.string(), name, module->m_source);

    // Instantiate lexer
    Lexer lexer(module->m_source);
    auto ttree = lexer.tokenize();

    // Instantiate parser
    Parser parser(module->m_source, ttree, diags);
    auto ast = parser.parse();

    // Check for fatal compilation errors
    bool failed = diags.has_errors();
    if (failed)
        goto error;

    {
        // Instantiate IR builder
        IRBuilder ir_builder(module, ast, diags);
        module->m_ir = ir_builder.build();

        // Check for errors during IR building
        failed = diags.has_errors();
        if (failed)
            goto error;

        // Map intermediate representation nodes to definitions
        for (const auto& node: module->m_ir) {
            // Check if the node has an identity
            if (auto symbol = node->get_symbol()) {
                module->m_defs[*symbol] = Def::from(manager, node);
            }
        }

        // Build executable
        Executable* exe = Executable::build_from_ir(module, diags, module->m_ir);
        module->m_exe = exe;

        // Check for the abscence of the no execution flag
        if ((flags & ModuleFlags::NO_EXECUTION) == 0) {
            // Initialize the virtual machine
            VirtualMachine vm(module, exe);

            // Check for debug flag
            if (flags & ModuleFlags::LAUNCH_DEBUGGER) {
                Debugger dbg(vm);
                dbg.register_default_commands();
                dbg.start();
            } else {
                // Run VM in contiguous execution mode
                vm.execute();
            }
        }
    }

error:
    // Handle diagnostics
    diags.emit();
    diags.clear();

    if (flags & ModuleFlags::DUMP_TTREE)
        std::cout << std::format("({}) ", module->m_name) << to_string(ttree) << "\n";
    if (flags & ModuleFlags::DUMP_AST)
        std::cout << std::format("({}) ", module->m_name) << to_string(ast) << "\n";
    if (flags & ModuleFlags::DUMP_IR)
        std::cout << std::format("({}) ", module->m_name)
                  << to_string(manager.symbol_table(), module->m_ir) << "\n";
    if (flags & ModuleFlags::DUMP_EXE)
        std::cout << std::format("({}) ", module->m_name)
                  << (module->m_exe ? module->m_exe->to_string() : "<executable error>")
                  << "\n";
    if (flags & ModuleFlags::DUMP_DEFTABLE)
        std::cout << std::format("({}) ", module->m_name)
                  << to_string(module->m_manager.symbol_table(), module->m_defs);

    // Handle failed compilation
    if (failed) {
        for (Module* module = importee; module != nullptr; module = module->m_importee)
            spdlog::info(std::format("Imported by module '{}'", module->m_name));
        if ((flags & (ModuleFlags::DUMP_TTREE | ModuleFlags::DUMP_AST |
                      ModuleFlags::DUMP_IR | ModuleFlags::DUMP_EXE)) != 0u)
            spdlog::warn("Dump may be invalid due to compilation failure");
    } else {
        manager.pop_import();
        return module;
    }
    manager.pop_import();
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

    std::filesystem::path path;
};

struct ModuleCandidate
{
    ModuleInfo::Kind kind;
    std::string name;
};

static std::optional<ModuleInfo> resolve_import_path(
    const std::filesystem::path& root,
    const via::QualName& path,
    const via::ModuleManager& manager
)
{
    via::debug::require(!path.empty(), "bad import path");

    auto path_slice = path;
    auto& module_name = path_slice.back();
    path_slice.pop_back();

    // Lambda to try candidates in a given base path
    auto try_dir_candidates =
        [&](const std::filesystem::path& dir) -> std::optional<ModuleInfo> {
        std::filesystem::path path = dir;
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

        auto try_path = [&](const std::filesystem::path& candidate,
                            ModuleInfo::Kind kind) -> std::optional<ModuleInfo> {
            if (std::filesystem::exists(candidate) &&
                std::filesystem::is_regular_file(candidate))
                return ModuleInfo{.kind = kind, .path = candidate};
            else
                return std::nullopt;
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

        return std::nullopt;
    };

    for (const auto& import_path: manager.get_import_paths()) {
        if (auto result = try_dir_candidates(import_path)) {
            return result;
        }
    }

    return std::nullopt;
}

std::optional<const via::Def*> via::Module::lookup(via::SymbolId symbol)
{
    if (auto it = m_defs.find(symbol); it != m_defs.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::expected<via::Module*, std::string>
via::Module::import(const QualName& path, const ast::StmtImport* ast_decl)
{
    auto module = resolve_import_path(m_path, path, m_manager);
    if (!module.has_value()) {
        return std::unexpected(std::format("Module '{}' not found", to_string(path)));
    }

    if ((m_perms & ModulePerms::IMPORT) == 0u) {
        return std::unexpected("Current module lacks import capabilties");
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
