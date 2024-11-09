#ifndef VIA_VM_H
#define VIA_VM_H

#include "common.h"
#include "core.h"
#include "opcode.h"
#include "bytecode.h"
#include "register.h"
#include "stack.h"
#include "gc.h"
#include "types.h"

#include "util/modifiable_once.h"
#include "util/callable_once.h"
#include "util/debounce.h"

#include "magic_enum.hpp"

namespace via
{

namespace VM
{

struct VMState
{
    bool is_running;
    std::string exit_message;
    int exit_code;

    std::unordered_map<std::string, bool> fflags;

    VMState() 
        : is_running(false)
        , exit_message("")
        , exit_code(0)
    {
        fflags = {
            {"FFLAG_ABRT", false},  // Abort execution in the next dispatch
            {"FFLAG_SKIP", false}   // Skip instruction in the next dispatch (debounces)
        };
    }
};

class VirtualMachine
{
    VMState state;
    Instruction* ip;
    Instruction* iphead;
    Instruction* ipend;

    RegisterAllocator regp;
    GarbageCollector gc;
    VMStack stack;
    Stack<Instruction*> callstack;
    StackFrame global;
    std::unordered_map<std::string_view, Instruction*> labels;

    int execute();

    inline via_Value interpret_operand(const Operand& o)
    {
        switch (o.type)
        {
        case Operand::OType::Number:
            return via_Value(o.num_val);
        case Operand::OType::Bool:
            return via_Value(o.bool_val);
        case Operand::OType::String:
            return via_Value(o.str_val);
        default:
            std::cout << "cannot interpret operand of type " << magic_enum::enum_name(o.type) << "\n";
            break;
        }

        return via_Value();
    }

    inline void set_exit_data(const int& e, const std::string& m)
    {
        state.exit_code = e;
        state.exit_message = m;
    }

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

    inline bool is_valid_jump_address(Instruction* addr) const
    {
        return (addr <= ipend) && (addr >= iphead);
    }

    inline void flushargs()
    {
        regp.flush(Register::RType::AR);
        regp.flush(Register::RType::SELFR);
        return;
    }

    inline void flushret()
    {
        regp.flush(Register::RType::RR);
        return;
    }

    inline void scopebegin()
    {
        stack.push();
        return;
    }

    inline void scopeend()
    {
        stack.pop();
        return;
    }

public:

    VirtualMachine(const std::vector<Instruction>& pipeline)
        : state(VMState())
        , regp(RegisterAllocator())
        , gc(GarbageCollector())
        , stack(VMStack(gc))
        , global(StackFrame(gc))
    {
        iphead = new Instruction[pipeline.size()];
        ip = iphead;
        ipend = iphead + pipeline.size();
        std::copy(pipeline.begin(), pipeline.end(), ip);
    }

    ~VirtualMachine() {
        gc.collect();
        delete[] iphead;
    }

    void init();
    bool is_running() { return state.is_running; }

    inline void set_fflag(const char* id, bool val)
    {
        auto fflag = state.fflags.find(std::string(id));

        VIA_ASSERT(fflag != state.fflags.end(),
            std::format("Unknown fflag '{}'", id).c_str());

        fflag->second = val;
        return;
    }

    inline bool get_fflag(const char* id)
    {
        auto fflag = state.fflags.find(std::string(id));

        VIA_ASSERT(fflag != state.fflags.end(),
            std::format("Unknown fflag '{}'", id).c_str());

        return fflag->second;
    }

    inline via_String tostring(const via_Value& v)
    {
        switch (v.type)
        {
        case via_Value::VType::String:
            return v.str_val;
        case via_Value::VType::Number: {
            auto str = strdup(std::to_string(v.num_val).c_str());
            gc.add(str);
            return str;
        }
        case via_Value::VType::Bool:
            return const_cast<char*>(v.bool_val ? "true" : "false");

        default:
            break;
        }

        return const_cast<char*>("nil");
    }

    inline via_Bool tobool(const via_Value& v)
    {
        switch (v.type)
        {
        case via_Value::VType::Nil:
            return false;
        case via_Value::VType::Bool:
            return v.bool_val;
        default:
            break;
        }

        return true;
    }

    inline via_Variant<via_Number, via_Nil> tonumber(const via_Value& v)
    {
        switch (v.type)
        {
        case via_Value::VType::String:
            return std::stod(v.str_val);
        case via_Value::VType::Bool:
            return v.bool_val ? 1.0f : 0.0f;
        default:
            break;
        }

        return via_Nil();
    }

    inline void rset(const Register& r, const via_Value& v)
    {
        *regp.get<via_Value>(r) = v;
        return;
    }

    inline via_Value& rget(const Register& r)
    {
        static auto val = *regp.get<via_Value>(r);
        return val;
    }

    inline via_Value* rget_address(const Register& r)
    {
        auto addr = regp.get<via_Value>(r);
        return addr;
    }

    inline void setl(const char* id, const via_Value& v)
    {
        stack.top().set_local(id, v);
        return;
    }

    inline via_Value& getl(const char* id)
    {
        static auto val = stack.top().get_local(id);
        return val;
    }

    inline via_Value& loadl(const char* id, const Register& r)
    {
        static auto val = getl(id);
        auto addr = rget_address(r);
        *addr = val;
        return val;
    }

    inline void gcadd(void* p)
    {
        gc.add(p);
        return;
    }

    inline void gccol()
    {
        gc.collect();
        return;
    }

    inline void call(const via_Func& f)
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
        callstack.push(ipc);
        flushret();
        scopebegin();
        jmpto(f.address + 1);

        return;
    }

    inline void ret()
    {
        if (callstack.is_empty())
        {
            set_exit_data(1, "Callstack underflow");
            set_fflag("FFLAG_ABRT", true);
            return;
        }

        // Return address
        auto ra = callstack.top();
        callstack.pop();

        scopeend();
        flushargs();
        jmpto(ra + 1);

        return;
    }

    inline void tinsert(via_Table*& t, const via_Value& v)
    {
        t->set(via_TableKey {
            .type = via_TableKey::KType::Number,
            .num_val = static_cast<via_Number>(t->data.size())
        }, v);

        return;
    }

    inline void tset(via_Table*& t, const via_TableKey& k, const via_Value& v)
    {
        t->set(k, v);
        return;
    }

    inline via_Value& tget(via_Table*& t, const via_TableKey& k)
    {
        return t->get(k);
    }

    inline via_Value& tload(via_Table*& t, const via_TableKey& k, const Register& r)
    {
        static auto v = tget(t, k);
        rset(r, v);
        return v;
    }

    inline void tcallm(via_Table*& t, const via_TableKey& k)
    {
        auto at = t->get(k);

        if (at.type != via_Value::VType::Func)
        {
            set_exit_data(1, "Attempt to call a non-function method");
            set_fflag("FFLAG_ABRT", true);
            return;
        }

        rset(Register {
            .type = Register::RType::SELFR,
            .offset = 0
        }, via_Value(*t));

        call(*at.fun_val);

        return;
    }

    inline void tfreeze(via_Table*& t)
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

    inline bool tisfrozen(via_Table*& t)
    {
        return t->is_frozen.get();
    }

    inline size_t len(const via_Value& v)
    {
        if (v.type == via_Value::VType::String)
        {
            return strlen(v.str_val);
        }
        else if (v.type == via_Value::VType::Table)
        {
            return v.tbl_val->data.size();
        }

        return -1;
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
            return v0.bool_val == v1.bool_val;
        case Ty::Nil:
            return true;
        case Ty::Number:
            return v0.num_val == v1.num_val;
        case Ty::Ptr:
            return v0.ptr_val == v1.ptr_val;
        case Ty::String:
            return !strcmp(v0.str_val, v1.str_val);
        case Ty::CFunc:
        case Ty::Func:
        case Ty::Table:
        case Ty::TableKey:
        case Ty::Vector:
            return false;
        default:
            break;
        }

        return false;
    }

    inline via_String vtype(const via_Value& v)
    {
        auto enum_name = magic_enum::enum_name(v.type);
        auto str = strdup(std::string(enum_name).c_str());

        gcadd(str);

        return str;
    }

    inline via_String vtypeof(const via_Value& v)
    {
        auto prim = vtype(v);

        if (strcmp(prim, ""))
        {
            return prim;
        }

        // TODO
        return const_cast<char*>("<complex-type>");
    }
};

} // namespace VM
    
} // namespace via

#endif // VIA_VM_H
