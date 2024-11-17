/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "bytecode.h"
#include "common.h"
#include "core.h"
#include "gc.h"
#include "global.h"
#include "opcode.h"
#include "register.h"
#include "stack.h"
#include "state.h"
#include "types.h"

#include "Utils/callable_once.h"
#include "Utils/debounce.h"
#include "Utils/modifiable_once.h"

namespace via::VM
{

class VirtualMachine
{
public:
    VirtualMachine(const std::vector<Instruction> &pipeline /* This has to be a constant ref */)
    {
        ihp = new Instruction[pipeline.size()]; // Allocate ihp    (Instruction head pointer)
        ip  = ihp;                              // Initialize ip   (Instruction pointer)
        ibp = ihp + pipeline.size();            // Initialize ibp  (Instruction base pointer)
        // ? There might be a more optimized way to do this
        std::copy(pipeline.begin(), pipeline.end(), ip); // Copy instructions into the instruction pipeline
    }

    ~VirtualMachine()
    {
        gccol();       // Final GC invocation, technically unnecessary because GC destructor already does this
        ip  = nullptr; // Invalidate ip
        ibp = nullptr; // Invalidate ibp
        delete[] ihp;  // Invalidate ihp
    }

    // Initializes VM code execution
    // Terminates the program if called twice
    void init();

    // Returns wether if the VM is running
    constexpr bool is_running() const noexcept
    {
        return m_state.is_running;
    }

    //* Internal

    // Manually sets the VM exit data
    // ! Internal usage only, made public for libraries
    inline void set_exit_data(int exit_code, const std::string &exit_message) noexcept
    {
        m_state.exit_code    = exit_code;
        m_state.exit_message = exit_message;
        return;
    }

    // Sets a fast flag to the given value
    // Terminates if the flag doesn't exist
    inline void set_fflag(FFlag flag, int val) noexcept
    {
        m_state.fflags.set(static_cast<int>(flag), val);
        return;
    }

    // Returns the given fast flags value
    // Terminates if the flag doesn't exist
    inline constexpr int get_fflag(FFlag flag) const noexcept
    {
        return m_state.fflags.test(static_cast<int>(flag));
    }

    // Appends a pointer to the garbage collector free list
    // The pointer should be malloc-ed, otherwise this has undefined behavior
    // ! The value cannot be removed from the free list!
    template <typename T>
    inline void gcadd(T p) noexcept
    {
        m_gc.add(p);
        return;
    }

    // Invokes garbage collection
    // Should be called in intervals
    inline void gccol() noexcept
    {
        m_gc.collect();
        return;
    }

    // Loads a static library to the global environment
    // If called during runtime, it will terminate the VM
    inline void loadlib(const char *id, const via_Value &lib) noexcept
    {
        if (is_running())
        {
            set_exit_data(1, "Attempt to load library during runtime");
            set_fflag(FFlag::ABRT, true);
            return;
        }

        if (gget(id).type != ValueType::Nil)
        {
            set_exit_data(1, std::format("Attempt to load library '{}' twice", id));
            set_fflag(FFlag::ABRT, true);
            return;
        }

        gset(id, lib);
        return;
    }

    // Asserts a condition, terminates the VM if not met
    inline void vm_assert(bool cond, const std::string &err) noexcept
    {
        if (!cond)
        {
            set_exit_data(1, std::format("VM assertion failed: {}", err));
            set_fflag(FFlag::ABRT, true);
        }
    }

    // Throws an unrecoverable error that terminates the VM
    inline void
    fatalerr(const std::string &err) noexcept // Yes, this is a noexcept function because the VM doesn't use exceptions
    {
        std::cerr << err << "\n";
        set_exit_data(1, std::format("User error: {}", err));
        set_fflag(FFlag::ABRT, true);
        return;
    }

    //* Register operations

    // Sets all argument registers to Nil
    // ! This can mess up the program control flow, use carefully
    inline void flushargs() noexcept
    {
        m_ralloc.flush(RegisterType::AR);
        m_ralloc.flush(RegisterType::SR);
        return;
    }

    // Sets all return registers to Nil
    // ! This can mess up the program control flow, use carefully
    inline void flushret() noexcept
    {
        m_ralloc.flush(RegisterType::RR);
        return;
    }

    // Sets register <r> to the given value <v>
    template <typename T = via_Value>
    inline void rset(const Register &r, const T &v) noexcept
    {
        *m_ralloc.get<T>(r) = v;
        return;
    }

    // Returns the value of register <r>
    template <typename T = via_Value>
    inline constexpr T rget(const Register &r) noexcept
    {
        return *rget_address<T>(r);
    }

    // Returns the memory address of register <r>
    template <typename T = via_Value>
    inline constexpr T *rget_address(const Register &r) noexcept
    {
        T *addr = m_ralloc.get<T>(r);
        return addr;
    }

    // Compares two registers types and offsets
    inline constexpr bool ris(const Register &r0, const Register &r1) noexcept
    {
        return r0.type == r1.type && r0.offset == r1.offset;
    }

    // Compares the value of two registers and returns wether if they are equivalent
    inline constexpr bool rcmp(const Register &r0, const Register &r1) noexcept
    {
        // Early return if registers are equivalent
        if (&r0 == &r1)
        {
            return true;
        }

        const via_Value &v0 = rget(r0);
        const via_Value &v1 = rget(r1);

        // Early type mismatch check
        if (v0.type != v1.type)
        {
            return false;
        }

        // Common types first to improve branching efficiency
        switch (v0.type)
        {
        case ValueType::Number:
            return v0.num == v1.num;
        case ValueType::Bool:
            return v0.boole == v1.boole;
        case ValueType::String:
            if (v0.str && v1.str)
            {
                if (strlen(v0.str) != strlen(v1.str))
                {
                    return false;
                }
                return !strcmp(v0.str, v1.str);
            }
            return v0.str == v1.str;
        case ValueType::Nil:
            return true; // Nil values are always equal
        case ValueType::Ptr:
            return v0.ptr == v1.ptr;
        default:
            return false; // Unique objects (CFunc, Func, Table, TableKey) are never equal
        }

        // If the type isn't matched, return false by default
        return false;
    }

    //* Stack/Global operations

    // Creates a new global variable with identifier <k> and value <v>
    // Terminates the VM if the identifier already exists in the global enviornment
    inline void gset(const char *k, via_Value v) noexcept
    {
        // Since there's no way for the `set_global` function to signal an error
        // I made it so that it returns a success code, which is a reverse boolean
        int success = m_global.set_global(k, v);

        if (success != 0)
        {
            set_exit_data(1, std::format("Global '{}' already exists", k));
            set_fflag(FFlag::ABRT, true);
        }

        return;
    }

    // Returns the value of identifier <k> if found in the global enviornment
    inline via_Value gget(const char *k) noexcept
    {
        return m_global.get_global(k);
    }

    // Loads and returns the value of <k> into register <r> if it's found in the global enviornment
    inline void gload(const char *k, const Register &r) noexcept
    {
        via_Value v = gget(k);
        rset(r, v);
        return;
    }

    // Sets the value of <id> to value <v> in the current stack frame
    inline void lset(const char *id, via_Value v)
    {
        m_stack.top().set_local(id, v);
        return;
    }

    // Returns the value of <id> if found in the current stack frame
    inline via_Value lget(const char *id)
    {
        via_Value val = m_stack.top().get_local(id);
        return val;
    }

    // Loads and returns the value of <id> into register <r> if found in the current stack frame
    inline via_Value lload(const char *id, const Register &r) noexcept
    {
        via_Value val = lget(id);
        rset(r, val);
        return val;
    }

    //* Value operations

    // Returns a value that contains a via_String that represents the stringified version of <v>
    // ! The return value is guaranteed to be a via_String
    inline via_Value &vtostring(via_Value &val)
    {
        if (val.type == ValueType::String)
        {
            return val;
        }

        switch (val.type)
        {
        case ValueType::Number:
        {
            via_String str = strdup(std::to_string(val.num).c_str());
            val.str        = str;
            break;
        }
        case ValueType::Bool:
        {
            via_String str = const_cast<char *>(val.boole ? "true" : "false");
            val.str        = str;
            break;
        }
        case ValueType::Table:
        {
            via_String str = strdup(std::format("table {}", static_cast<const void *>(val.tbl)).c_str());
            val.str        = str;
            break;
        }
        case ValueType::Func:
        {
            via_String str = strdup(std::format("function {}", static_cast<const void *>(val.fun)).c_str());
            val.str        = str;
            break;
        }
        case ValueType::CFunc:
        {
            via_String str = strdup(std::format("cfunction {}", reinterpret_cast<void *>(val.cfun)).c_str());
            val.str        = str;
            break;
        }
        default:
            // Have to use `strdup` to duplicate the string
            // This is because "nil" is stack allocated and once it goes out of scope
            // It will be a dangling pointer
            // Basically fast dynamic string allocation :)
            val.str = strdup("nil");
            break;
        }

        val.type = ValueType::String;
        return val;
    }

    // Returns the truthiness of value <v>
    // ! Guaranteed to be a via_Bool
    inline via_Value &vtobool(via_Value &val)
    {
        if (val.type == ValueType::Bool)
        {
            return val;
        }

        switch (val.type)
        {
        case ValueType::Nil:
            val.boole = false;
            break;
        default:
            val.boole = true;
            break;
        }

        val.type = ValueType::Bool;
        return val;
    }

    // Returns the number representation of value <v>
    // ! Returns Nil if impossible, unlike `vtostring` or `vtobool`
    inline via_Value &vtonumber(via_Value &val)
    {
        if (val.type == ValueType::Number)
        {
            return val;
        }

        switch (val.type)
        {
        case ValueType::String:
            val.num = std::stod(val.str);
            break;
        case ValueType::Bool:
            val.num = val.boole ? 1.0f : 0.0f;
            break;
        default:
            val.nil = nullptr;
            return val;
        }

        val.type = ValueType::Number;
        return val;
    }

    // Calls a via_Func type, terminates if uncallable
    // Uses the FASTCALL convention
    // Arguments and return values are loaded onto registers of those respective types
    // For example, argument register #1 would be;
    // { RegisterType::AR, 0 }
    // This allows for extremely fast function calls and an alternative to stack based calling conventions
    inline void callf(const via_Func &f)
    {
        if (!is_valid_jump_address(f.address))
        {
            set_exit_data(1, "Invalid function jump address");
            set_fflag(FFlag::ABRT, true);
            return;
        }

        if (f.address->op == OpCode::FUNC)
        {
            set_exit_data(1, "Function jump address points to non-function opcode");
            set_fflag(FFlag::ABRT, true);
            return;
        }

        auto ipc = ip;
        m_stack.push(StackFrame(ipc, m_gc));
        flushret();
        jmpto(f.address + 1);

        return;
    }

    // Calls a C function pointer
    // This allows for a more flexible datatype, and avoids another layer of pointers in the via_Value union
    // C functions in via must return void and are called with VirtualMachine *
    // Type: void(*)(VirtualMachine *)
    inline void callc(const via_CFunc &cf)
    {
        auto ipc = ip;
        m_stack.push(StackFrame(ipc, m_gc));
        cf(this);
        m_stack.pop();
        return;
    }

    // Generalized call interface
    // Used to call a nil-able via_Value
    // Terminates if the value is not callable
    // Callable types include;
    // - via_Func
    // - via_CFunc
    // - via_Table (if __call method is present)
    inline void call(const via_Value &v)
    {
        if (v.type == ValueType::Func)
            callf(*v.fun);
        else if (v.type == ValueType::CFunc)
            callc(*v.cfun);
        else if (v.type == ValueType::Table)
        {
            via_Value call_mm = tget(v.tbl, "__call");
            call(call_mm);
        }
        else
        {
            set_exit_data(1, std::format("Attempt to call a {} value", vtype(v).str));
            set_fflag(FFlag::ABRT, true);
        }

        return;
    }

    // Returns the length of value <v>, -1 if impossible
    inline size_t len(const via_Value &v)
    {
        if (v.type == ValueType::String)
        {
            return strlen(v.str);
        }
        else if (v.type == ValueType::Table)
        {
            return v.tbl->data.size();
        }

        return -1;
    }

    // Returns the primitive type of value <v>
    inline via_Value vtype(const via_Value &v)
    {
        auto enum_name = ENUM_NAME(v.type);
        auto str       = strdup(std::string(enum_name).c_str());
        gcadd(str);
        return via_Value(str);
    }

    // Returns the complex type of value <v>
    // Practically the same as `vtype()`, but returns
    // the `__type` value if the given table has one
    inline via_Value vtypeof(const via_Value &v)
    {
        if (v.type == ValueType::Table)
        {
            auto t  = v.tbl;
            auto ty = t->get(via_TableKey { .type = via_TableKey::KType::String, .str = const_cast<char *>("__type") });

            if (ty.type == ValueType::Nil)
            {
                return vtype(v);
            }

            return via_Value(ty.str);
        }

        return vtype(v);
    }

    // Copies a value
    inline via_Value vcopy(const via_Value &v)
    {
        via_Value copy = via_Value(v);
        return copy;
    }

    // Copies a value onto the heap
    // ! Not garbage collected
    inline via_Value *vcopyheap(const via_Value &v)
    {
        via_Value *copy = new via_Value(v);
        return copy;
    }

    // Utility function for quick table indexing
    // Returns the value of key <k> if present in table <t>
    inline via_Value tget(via_Table *t, const char *k)
    {
        return t->get({ .type = via_TableKey::KType::String, .str = const_cast<char *>(k) });
    }

    // Assigns the given value <v> to key <k> in table <t>
    inline void tset(via_Table *t, const char *k, via_Value v) // Utility function for quick table index assignment
    {
        t->set({ .type = via_TableKey::KType::String, .str = const_cast<char *>(k) }, v);
        return;
    }

    // Pushes value <v> to the back of table <t>
    inline void tinsert(via_Table *t, const via_Value &v)
    {
        t->set(via_TableKey { .type = via_TableKey::KType::Number, .num = static_cast<via_Number>(t->data.size()) }, v);
        return;
    }

    // Loads the value of key <k> in table <t> into register <r>, if present in table
    inline via_Value tload(via_Table *t, const via_TableKey &k, const Register &r)
    {
        via_Value v = t->get(k);
        rset(r, v);
        return v;
    }

    // Calls the value of key <k> in table <t>, if callable
    // Uses the `call` method internally
    inline void tcallm(via_Table *t, const via_TableKey &k)
    {
        auto at = t->get(k);
        rset(Register { .type = RegisterType::SR, .offset = 0 }, via_Value(*t));
        call(at);
        return;
    }

    // Freezes table <t>
    // Terminates the VM if table <t> is already frozen
    inline void tfreeze(via_Table *t)
    {
        if (tisfrozen(t))
        {
            set_exit_data(1, "Attempt to freeze table twice");
            set_fflag(FFlag::ABRT, true);
            return;
        }

        t->is_frozen.set(true);
        return;
    }

    // Returns wether if table <t> is frozen
    inline bool tisfrozen(via_Table *t)
    {
        return t->is_frozen.get();
    }

private:
    // I don't particularly like doing this
    // However, it makes the code a lot cleaner
    using VMStack = Stack<StackFrame>;
    using Labels  = std::unordered_map<std::string_view, Instruction *>;

    VMState m_state;  // State object for the VM
    Instruction *ip;  // Instruction pointer
    Instruction *ihp; // Instruction list head
    Instruction *ibp; // Instruction list base

    Global m_global;            // VM Global environment
    VMStack m_stack;            // VM Stack
    Labels m_labels;            // VM Label address table (LAT)
    GarbageCollector m_gc;      // VM Garbage collector
    RegisterAllocator m_ralloc; // VM Register allocator

private:
    //* Entry point
    // Located in vm.cpp
    int execute();

    //* Control flow
    inline void jmpto(Instruction *i)
    {
        if (!is_valid_jump_address(i))
        {
            set_exit_data(1, "Illegal jump: jump  address out of bounds");
            set_fflag(FFlag::ABRT, true);
            return;
        }

        ip = i;
        return;
    }

    inline void jmp(int offset)
    {
        Instruction *target_ip = ip + offset;
        jmpto(target_ip);
        return;
    }

    /*
    ! This function should not be called outside corresponding stack frames
    ! This WILL cause a stack underflow if used incorrectly
    ! May even cause undefined behavior if somehow the return_address pointer
    ! is within the bounds of the ip pipeline but is dangling/invalid
    // DO NOT ASK ME HOW THAT COULD HAPPEN
    */
    inline void ret()
    {
        if (m_stack.is_empty())
        {
            set_exit_data(1, "Callstack underflow");
            set_fflag(FFlag::ABRT, true);
            return;
        }

        // Return address
        auto ra = m_stack.top().return_address;
        m_stack.pop();

        if (!is_valid_jump_address(const_cast<Instruction *>(ra)))
        {
            set_exit_data(1, "invalid return address");
            set_fflag(FFlag::ABRT, true);
            return;
        }

        flushargs();
        jmpto(const_cast<Instruction *>(ra) + 1);

        return;
    }

    //* Utility
    inline bool is_valid_jump_address(Instruction *addr) const noexcept
    {
        return (addr >= ihp) && (addr <= ibp);
    }

    inline via_Value interpret_operand(const Operand &o)
    {
        switch (o.type)
        {
        case OperandType::Number:
            return via_Value(o.num);
        case OperandType::Bool:
            return via_Value(o.boole);
        case OperandType::String:
            return via_Value(o.str);
        default:
            set_exit_data(1, std::format("Cannot interpret operand '{}' as a data type", ENUM_NAME(o.type)));
            set_fflag(FFlag::ABRT, true);
            break;
        }

        return via_Value();
    }

    inline void save_state()
    {
    }
};

} // namespace via::VM
