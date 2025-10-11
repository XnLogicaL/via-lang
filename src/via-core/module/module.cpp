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
#include <iomanip>
#include <iostream>
#include <replxx.hxx>
#include "debug.hpp"
#include "ir/builder.hpp"
#include "manager.hpp"
#include "source.hpp"
#include "support/ansi.hpp"
#include "support/memory.hpp"
#include "support/os/dl.hpp"
#include "symbol.hpp"
#include "vm/instruction.hpp"
#include "vm/machine.hpp"
#include "vm/value.hpp"

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

    // Check for definition table dump flag
    if (flags & ModuleFlags::DUMP_DEFTABLE) {
        // Dump definition table header
        std::cout << ansi::format(
                         std::format("[deftable .{}]", name),
                         ansi::Foreground::YELLOW,
                         ansi::Background::NONE,
                         ansi::Style::BOLD
                     )
                  << "\n";

        // Dump definition table entries
        for (const auto& def: module->m_defs) {
            std::cout << "  " << def.second->to_string() << "\n";
        }
    }

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
    Parser parser(*file, ttree, diags);
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
            if (flags & ModuleFlags::DEBUG) {
                start_debugger(vm);
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

    // Handle flags
    if (flags & ModuleFlags::DUMP_TTREE)
        std::cout << debug::to_string(ttree) << "\n";
    if (flags & ModuleFlags::DUMP_AST)
        std::cout << debug::to_string(ast) << "\n";
    if (flags & ModuleFlags::DUMP_IR)
        std::cout << debug::to_string(manager.symbol_table(), module->m_ir) << "\n";
    if (flags & ModuleFlags::DUMP_EXE)
        std::cout << (module->m_exe ? module->m_exe->to_string() : "<null-executable>")
                  << "\n";
    if (flags & ModuleFlags::DUMP_DEFTABLE) {
        std::cout << ansi::format(
                         std::format("[deftable .{}]", name),
                         ansi::Foreground::YELLOW,
                         ansi::Background::NONE,
                         ansi::Style::BOLD
                     )
                  << "\n";

        for (const auto& def: module->m_defs) {
            std::cout << "  " << def.second->to_string() << "\n";
        }
    }

    // Handle failed compilation
    if (failed) {
        // Dump importee
        // TODO: Replace this with an import chain rather than a single module
        for (Module* module = importee; module != nullptr; module = module->m_importee)
            spdlog::info(std::format("Imported by module '{}'", module->m_name));

        // Print warning message about compilation failure
        if ((flags & (ModuleFlags::DUMP_TTREE | ModuleFlags::DUMP_AST |
                      ModuleFlags::DUMP_IR | ModuleFlags::DUMP_EXE)) != 0u) {
            spdlog::warn("Dump may be invalid due to compilation failure");
        }
    } else {
        // Pop import stack
        manager.pop_import();
        return module;
    }

    // Pop import stack
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
    auto try_dir_candidates = [&](const std::filesystem::path& dir
                              ) -> std::optional<ModuleInfo> {
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

void via::Module::start_debugger(VirtualMachine& vm) noexcept
{
    // Initialize the REPL
    replxx::Replxx repl;

    // Print the welcome message
    spdlog::info("Starting interactive VM debugger...\n"
                 "  > step      steps the interpreter\n"
                 "  > raise     raise default error\n"
                 "  > pc        dumps the interpreter program counter\n"
                 "  > regs      dumps the interpreter register buffer\n"
                 "  > stack     dumps the interpreter stack\n");

    vm.set_int_hook([](VirtualMachine* vm, Interrupt in, void* arg) {
        std::cout << "Machine interrupted\n";
        std::cout << " code: 0x" << std::hex << size_t(in) << std::dec;
        std::cout << " " << std::format("({})\n", via::to_string(in));

        if (in == Interrupt::ERROR) {
            auto* error = reinterpret_cast<ErrorInt*>(arg);
            std::cout << " error info:\n";
            std::cout << "  msg:  " << error->msg << "\n";
            std::cout << "  out:  " << (void*) error->out << "\n";
            std::cout << "  fp:   " << (void*) error->fp << "\n";
            std::cout << "  pc:   " << (void*) error->pc << "\n";
        }
    });

    // Start the REPL loop
    while (true) {
        // Retrieve input from the REPL
        const char* cinput = repl.input("> ");
        // Check for empty input
        if (cinput == nullptr) {
            break;
        }

        std::string input(cinput); // Input from the REPL
        Snapshot snapshot(&vm);    // Snapshot the VM state

        if (input == "step") { // Step option
            // Step the VM
            vm.execute_once();

            // Check for trailing halt instruction
            if (snapshot.program_counter->op == OpCode::HALT) {
                break;
            }
        } else if (input == "raise") {
            vm.raise("<repl-raised-error>", &std::cout);
        } else if (input == "pc") { // Program counter dump option
            // Print the program counter
            std::cout << "pc:   " << (void*) snapshot.program_counter << "\n";
            std::cout << "rel:  0x" << std::hex << std::right << std::setw(4)
                      << std::setfill('0') << snapshot.rel_program_counter * 8 << "\n";
            std::cout << snapshot.program_counter->to_string(
                             false,
                             snapshot.rel_program_counter
                         )
                      << "\n";
        } else if (input == "regs") { // Register dump option
            // Iterate over the registers and print their values
            for (size_t index = 0; const auto& ptr: snapshot.registers) {
                // Check if the register is filled
                if (ptr != nullptr) {
                    // Dump the register value
                    std::cout << std::format("R{} = {}\n", index, ptr->to_string());
                }
                index++;
            }
        } else if (input == "stack") { // Stack dump option
            // Dump the stack size
            std::cout << std::format("size: {}\n", snapshot.stack.size());

            // Check if the stack is not empty
            if (!snapshot.stack.empty()) {
                // Save the frame pointer and stack base
                const uintptr_t fp = snapshot.frame_ptr;
                const uintptr_t* stk_base;

                // Check if the frame pointer is valid
                if (fp != 0) {
                    // Update the stack base
                    stk_base = snapshot.stack.begin().base() + fp;

                    // Save stack local state
                    auto old_fp = (uintptr_t*) *(stk_base - 0);
                    auto ret_pc = (Instruction*) *(stk_base - 1);
                    auto flags = (uint64_t) *(stk_base - 2);
                    auto callee = (Value*) *(stk_base - 3);

                    // Dump stack local state
                    std::cout << "Frame @ " << fp << "\n";
                    std::cout << "  callee   = " << callee->to_string() << "\n";
                    std::cout << "  flags    = " << flags << "\n";
                    std::cout << "  ret_pc   = " << (const void*) ret_pc << "\n";
                    std::cout << "  old_fp   = " << (const void*) old_fp << "\n";
                } else {
                    // Update the stack base
                    stk_base = snapshot.stack.begin().base() - 1;
                }

                // Dump locals
                for (auto* ptr = stk_base + 1; ptr < snapshot.stack.end().base() - 1;
                     ++ptr) {
                    if (fp != 0) {
                        std::cout << "  ";
                    }

                    auto* val = (Value*) *ptr;
                    std::cout << std::format(
                        "local {} = {}\n",
                        ptr - stk_base - 1,
                        val->to_string()
                    );
                }

                std::cout << "raw stack dump:\n";
                for (size_t i = 0; uintptr_t ptr: snapshot.stack) {
                    std::cout << "[" << i++ << "] ";
                    std::cout << "0x" << std::hex << ptr << "\n";
                }
            }
        } else { // Invalid option
            // Echo the input
            std::cout << input << "\n";
        }
    }
}
