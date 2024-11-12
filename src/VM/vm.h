#ifndef VIA_VM_H
#define VIA_VM_H

#include "common.h"
#include "core.h"
#include "opcode.h"
#include "bytecode.h"
#include "register.h"
#include "stack.h"
#include "gc.h"
#include "state.h"
#include "global.h"
#include "types.h"

#include "util/modifiable_once.h"
#include "util/callable_once.h"
#include "util/debounce.h"

namespace via::VM
{

class VirtualMachine
{
private:

    VMState m_state;        // State object for the VM
    Instruction* ip;        // Instruction pointer
    Instruction* ip_s;      // Instruction list head
    Instruction* ip_e;      // Instruction list base

    Global m_global;            // VM Global environment
    Stack<StackFrame> m_stack;  // VM Stack
    RegisterAllocator m_ralloc; // VM Register allocator
    GarbageCollector m_gc;      // VM Garbage collector

    std::unordered_map<std::string_view, Instruction*> m_labels;

    int execute();

    //* Control flow
    inline void jmpto(Instruction* i)
    {
        if (!is_valid_jump_address(i))
        {
            set_exit_data(1, "Invalid jump address");
            set_fflag("FFLAG_ABRT", true);
            return;
        }

        ip = i;
        return;
    }

    inline void jmp(int offset)
    {
        Instruction* target_ip = ip + static_cast<int>(offset);

        if (!is_valid_jump_address(target_ip))
        {
            set_exit_data(1, "Illegal jump");
            set_fflag("FFLAG_ABRT", true);
            return;
        }

        ip = target_ip;
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
            set_fflag("FFLAG_ABRT", true);
            return;
        }

        // Return address
        auto ra = m_stack.top().return_address;
        m_stack.pop();

        flushargs();
        jmpto(const_cast<Instruction*>(ra) + 1);

        return;
    }

    //* Utility
    inline bool is_valid_jump_address(Instruction* addr) const
    {
        return (addr <= ip_e) && (addr >= ip_s);
    }

    inline via_Value interpret_operand(const Operand& o)
    {
        switch (o.type)
        {
        case Operand::OType::Number:
            return via_Value(o.num);
        case Operand::OType::Bool:
            return via_Value(o.boole);
        case Operand::OType::String:
            return via_Value(o.str);
        default:
            set_exit_data(1, std::format("Cannot interpret operand '{}' as a data type", ENUM_NAME(o.type)));
            set_fflag("FFLAG_ABRT", true);
            break;
        }

        return via_Value();
    }

public:

    VirtualMachine(const std::vector<Instruction>& pipeline)
    {
        ip_s = new Instruction[pipeline.size()];
        ip = ip_s;
        ip_e = ip_s + pipeline.size();
        //// std::copy(pipeline.begin(), pipeline.end(), ip);
    }

    ~VirtualMachine()
    {
        gccol(); // Final GC invocation, technically unnecessary because GC destructor already does this
        ip = nullptr;
        ip_e = nullptr;
        delete[] ip_s;
    }

    void init();
    bool is_running() { return m_state.is_running; }

    //* Internal
    inline void set_exit_data(const int& e, const std::string& m)
    {
        m_state.exit_code = e;
        m_state.exit_message = m;
        return;
    }

    inline void set_fflag(const char* id, int val)
    {
        auto fflag = m_state.fflags.find(std::string(id));

        VIA_ASSERT(fflag != m_state.fflags.end(),
            std::format("Unknown fflag '{}'", id).c_str());

        fflag->second = val;
        return;
    }

    inline int get_fflag(const char* id)
    {
        auto fflag = m_state.fflags.find(std::string(id));

        VIA_ASSERT(fflag != m_state.fflags.end(),
            std::format("Unknown fflag '{}'", id).c_str());

        return fflag->second;
    }

    inline void gcadd(void* p)
    {
        m_gc.add(p);
        return;
    }

    inline void gcaddheap(void *p)
    {
        m_gc.add_heap(p);
        return;
    }

    inline void gccol()
    {
        m_gc.collect();
        return;
    }

    inline via_Value loadlib(const via_String& id, via_Value& lib)
    {
        if (is_running())
        {
            set_exit_data(1, "Attempt to load library during runtime");
            set_fflag("FFLAG_ABRT", true);
            return;
        }

        if (gget(id).type != via_Value::VType::Nil)
        {
            set_exit_data(1, std::format("Attempt to load library '{}' twice", id));
            set_fflag("FFLAG_ABRT", true);
            return;
        }

        gset(id, lib);
        return;
    }

    inline void vm_assert(bool cond, std::string err)
    {
        if (!cond)
        {
            set_exit_data(1, std::format("VM assertion failed: {}", err));
            set_fflag("FFLAG_ABRT", true);
        }
    }
    
    inline void fatalerr(const std::string& err)
    {
        std::cerr << err << "\n";
        set_exit_data(1, std::format("User error: {}", err));
        set_fflag("FFLAG_ABRT", true);
        return;
    }

    //* Register operations
    inline void flushargs()
    {
        m_ralloc.flush(Register::RType::AR);
        m_ralloc.flush(Register::RType::SELFR);
        return;
    }

    inline void flushret()
    {
        m_ralloc.flush(Register::RType::RR);
        return;
    }

    inline void rset(const Register& r, via_Value v)
    {
        *m_ralloc.get<via_Value>(r) = v;
        return;
    }

    inline via_Value& rget(const Register& r)
    {
        static auto val = *m_ralloc.get<via_Value>(r);
        return val;
    }

    inline via_Value* rget_address(const Register& r)
    {
        auto addr = m_ralloc.get<via_Value>(r);
        return addr;
    }

    inline bool ris(const Register& r0, const Register& r1)
    {
        return r0.type == r1.type && r0.offset == r1.offset;
    }

    inline bool rcmp(const Register& r0, const Register& r1)
    {
        if (ris(r0, r1))
        {
            return true;
        }

        auto v0 = rget(r0);
        auto v1 = rget(r1);

        if (v0.type != v1.type)
        {
            return false;
        }

        using Ty = via_Value::VType;

        switch (v0.type)
        {
        case Ty::Bool:
            return v0.boole == v1.boole;
        case Ty::Nil:
            return true;
        case Ty::Number:
            return v0.num == v1.num;
        case Ty::Ptr:
            return v0.ptr == v1.ptr;
        case Ty::String:
            return !strcmp(v0.str, v1.str);
        case Ty::CFunc:
        case Ty::Func:
        case Ty::Table:
        case Ty::TableKey:
            return false;
        default:
            break;
        }

        return false;
    }

    //* Stack/Global operations
    inline void gset(const via_String& k, via_Value v)
    {
        int exit_code = m_global.set_global(k, v);

        if (exit_code != 0)
        {
            set_exit_data(1, std::format("Global '{}' already exists", k));
            set_fflag("FFLAG_ABRT", true);
        }

        return;
    }

    inline via_Value gget(const via_String& k)
    {
        return m_global.get_global(k);
    }

    inline void lset(const char* id, via_Value v)
    {
        m_stack.top().set_local(id, v);
        return;
    }

    inline via_Value& lget(const char* id)
    {
        static auto val = m_stack.top().get_local(id);
        return val;
    }

    inline via_Value& lload(const char* id, const Register& r)
    {
        static auto val = lget(id);
        auto addr = rget_address(r);
        *addr = val;
        return val;
    }

    //* Value operations
    inline via_Value vtostring(const via_Value &v)
    {
        switch (v.type)
        {
        case via_Value::VType::String:
            return via_Value(v.str);
        case via_Value::VType::Number: {
            auto str = strdup(std::to_string(v.num).c_str());
            gcadd(str);
            return via_Value(str);
        }
        case via_Value::VType::Bool:
            return via_Value(v.boole ? "true" : "false");
        case via_Value::VType::Table: {
            via_String str = strdup(std::format("table 0x{}", v.tbl->uid).c_str());
            gcadd(str);
            return via_Value(str);
        }
        default:
            break;
        }

        return via_Value("nil");
    }

    inline via_Value vtobool(const via_Value& v)
    {
        switch (v.type)
        {
        case via_Value::VType::Nil:
            return via_Value(false);
        case via_Value::VType::Bool:
            return via_Value(v.boole);
        default:
            break;
        }

        return via_Value(true);
    }

    inline via_Value vtonumber(const via_Value& v)
    {
        switch (v.type)
        {
        case via_Value::VType::String:
            return via_Value(std::stod(v.str));
        case via_Value::VType::Bool:
            return via_Value(v.boole ? 1.0f : 0.0f);
        default:
            break;
        }

        return via_Value();
    }

    // In via, the default calling convention is the FASTCALL convention
    // Arguments and return values are loaded onto registers of those respective types
    // For example, argument register #1 would be;
    // { Register::RType::AR, 0 }
    // This allows for extremely fast function calls and an alternative to stack based calling convetions
    inline void callf(const via_Func &f)
    {
        if (!is_valid_jump_address(f.address))
        {
            set_exit_data(1, "Invalid function jump address");
            set_fflag("FFLAG_ABRT", true);
            return;
        }

        if (f.address->op == OpCode::FUNC)
        {
            set_exit_data(1, "Function jump address points to non-function opcode");
            set_fflag("FFLAG_ABRT", true);
            return;
        }

        auto ipc = ip;
        m_stack.push(StackFrame(ipc, m_gc));
        flushret();
        jmpto(f.address + 1);

        return;
    }

    // via uses function pointers instead of std::function to represent C functions
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
    inline void call(const via_Value &v)
    {
        switch (v.type)
        {
        case via_Value::VType::Func:
            callf(*v.fun);
            return;

        case via_Value::VType::CFunc:
            callc(*v.cfun);
            return;
        
        default:
            break;
        }

        set_exit_data(1, std::format("Attempt to call a {} value", vtype(v).str));
        set_fflag("FFLAG_ABRT", true);
        return;
    }

    inline size_t len(const via_Value& v)
    {
        if (v.type == via_Value::VType::String)
        {
            return strlen(v.str);
        }
        else if (v.type == via_Value::VType::Table)
        {
            return v.tbl->data.size();
        }

        return -1;
    }

    inline via_Value vtype(const via_Value &v)
    {
        auto enum_name = ENUM_NAME(v.type);
        auto str = strdup(std::string(enum_name).c_str());
        gcadd(str);
        return via_Value(str);
    }

    inline via_Value vtypeof(const via_Value &v)
    {
        if (v.type == via_Value::VType::Table)
        {
            auto t = v.tbl;
            auto ty = t->get(via_TableKey {
                .type = via_TableKey::KType::String,
                .str = "__type"
            });

            if (ty.type == via_Value::VType::Nil)
            {
                return vtype(v);
            }

            return via_Value(ty.str);
        }
        
        return vtype(v);
    }

    inline via_Value &vcopy(const via_Value &v) // Copies a value
    {
        via_Value copy = via_Value(v);
        return copy;
    }

    inline via_Value *vcopyheap(const via_Value &v) // Copies a value onto the heap
    {
        via_Value *copy = new via_Value(v);
        return copy;
    }

    inline via_Value tget(via_Table *t, const via_String k) // Utility function for quick table indexing
    {
        return t->get({ .type = via_TableKey::KType::String, .str = k });
    }

    inline void tset(via_Table *t, const via_String k, via_Value v) // Utility function for quick table index assignment
    {
        t->set({ .type = via_TableKey::KType::String, .str = k }, v);
        return;
    }

    inline void tinsert(via_Table *t, const via_Value& v)
    {
        t->set(via_TableKey{
            .type = via_TableKey::KType::Number,
            .num = static_cast<via_Number>(t->data.size())
            }, v);

        return;
    }

    inline via_Value& tload(via_Table *t, const via_TableKey& k, const Register& r)
    {
        static auto v = t->get(k);
        rset(r, v);
        return v;
    }

    inline void tcallm(via_Table *t, const via_TableKey& k)
    {
        auto at = t->get(k);

        rset(Register{
            .type = Register::RType::SELFR,
            .offset = 0
            }, via_Value(*t));

        call(at);

        return;
    }

    inline void tfreeze(via_Table *t)
    {
        if (tisfrozen(t))
        {
            set_exit_data(1, "Attempt to freeze table twice");
            set_fflag("FFLAG_EXIT", true);
            return;
        }

        t->is_frozen.set(true);
        return;
    }

    inline bool tisfrozen(via_Table *t)
    {
        return t->is_frozen.get();
    }
};

} // namespace via::VM

#endif // VIA_VM_H
