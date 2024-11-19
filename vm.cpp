/* This file is a part of the via programming language at
 * https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

/*
 * !! WARNING !!
 * Unit testing is not allowed in this file!
 * This is due to the performance critical nature of it's contents
 */

#include "vm.h"

#include <cmath>
#include <cstring>

using namespace via::VM;

// Simple macro for exiting the VM
// Technically not necessary but makes it a tiny bit more readable
#define VM_EXIT() \
    { \
        goto exit; \
    }

// Same as VM_EXIT, for readability
#define VM_DISPATCH() \
    { \
        goto dispatch; \
    }

// Macro for loading the next instruction
// Has bound checks
#define VM_LOAD() \
    do \
    { \
        if (!is_valid_jump_address(ip + 1)) \
        { \
            set_exit_data(0, ""); \
            VM_EXIT(); \
        } \
        ip++; \
    } while (0)

// Macro that "signals" the VM has completed an execution cycle
#define VM_NEXT() \
    do \
    { \
        VM_LOAD(); \
        VM_DISPATCH(); \
    } while (0);

// Returns the operand at offset <off>
// ! Can cause undefined behavior
#define VM_OPND(off) (ip->operandv[off])

// Standard VM assertion
// Terminates VM if the assertion fails
// Contains debug information, such as file, line and a custom message
#define VM_ASSERT(cond, message) \
    do \
    { \
        if (!(cond)) \
        { \
            set_exit_data(1, std::format("VM_ASSERT(): {}\n in file {}, line {}", (message), __FILE__, __LINE__).c_str()); \
            VM_EXIT(); \
        } \
    } while (0)

// Macro for jumping to a jump address
// Immediately dispatches the VM
#define VM_JMPTO(to) \
    do \
    { \
        jmpto(to); \
        VM_DISPATCH(); \
    } while (0)

// Macro for jumping to an offset relative to the current instruction pointer
// position Immediately dispatches VM
#define VM_JMP(offset) \
    do \
    { \
        jmp(offset); \
        VM_DISPATCH(); \
    } while (0);

int VirtualMachine::execute()
{
    // Not necessary, but provides clarity
    VM_DISPATCH();

dispatch:
{
    /*

    if (get_fflag(FFlag::ABRT))
    {
        VM_EXIT();
    }

    if (get_fflag(FFlag::SKIP))
    {
        set_fflag(FFlag::SKIP, false);
        VM_NEXT();
    }

    VM_ASSERT(is_valid_jump_address(ip),
              std::format("Instruction pointer out of bounds (ip={}, ihp={},
    ibp={})", reinterpret_cast<uintptr_t>(ip), reinterpret_cast<uintptr_t>(ihp),
                          reinterpret_cast<uintptr_t>(ibp)));

    */

    switch (ip->op)
    {
    case OpCode::END:
    case OpCode::NOP:
        VM_NEXT();

    case OpCode::MOV:
    {
        Operand dst_r = VM_OPND(0);
        Operand src_r = VM_OPND(1);

        rset(dst_r.reg, rget(src_r.reg));
        rset(dst_r.reg, via_Value());

        VM_NEXT();
    }

    case OpCode::CPY:
    {
        Operand dst_r = VM_OPND(0);
        Operand src_r = VM_OPND(1);

        rset(dst_r.reg, rget(src_r.reg));

        VM_NEXT();
    }

    case OpCode::LOAD:
    {
        Operand dst_r = VM_OPND(0);
        Operand src_p = VM_OPND(1);
        /* TODO: Make this safe */
        rset(dst_r.reg, *reinterpret_cast<via_Value*>(static_cast<uintptr_t>(src_p.num)));

        VM_NEXT();
    }

    case OpCode::STORE:
    {
        Operand dst_p = VM_OPND(0);
        Operand src_r = VM_OPND(1);
        /* TODO: Make this safe */
        *reinterpret_cast<via_Value*>(static_cast<uintptr_t>(dst_p.num)) = rget(src_r.reg);

        VM_NEXT();
    }

    case OpCode::LI:
    {
        Operand dst_r = VM_OPND(0);
        via_Value val = interpret_operand(VM_OPND(1));

        rset(dst_r.reg, val);

        VM_NEXT();
    }

    case OpCode::PUSH:
    {
        Instruction* ipc = ip;
        m_stack.push(StackFrame(ipc, m_gc));
        VM_NEXT();
    }

    case OpCode::POP:
        m_stack.pop();
        VM_NEXT();

    case OpCode::SETLOCAL:
    {
        Operand id = VM_OPND(0);
        Operand val = VM_OPND(1);

        lset(id.ident, interpret_operand(val));

        VM_NEXT();
    }

    case OpCode::GETLOCAL:
    {
        Operand id = VM_OPND(0);
        Operand dst = VM_OPND(1);

        lload(id.ident, dst.reg);

        VM_NEXT();
    }

    case OpCode::SETGLOBAL:
    {
        Operand id = VM_OPND(0);
        Operand val = VM_OPND(1);

        gset(id.ident, interpret_operand(val));

        VM_NEXT();
    }

    case OpCode::GETGLOBAL:
    {
        Operand id = VM_OPND(0);
        Operand dst = VM_OPND(1);

        gload(id.ident, dst.reg);

        VM_NEXT();
    }

#define VM_BINOP(op) \
    { \
        Operand dst = VM_OPND(0); \
        Operand lhs = VM_OPND(1); \
        Operand rhs = VM_OPND(2); \
        via_Value lhs_n = rget(lhs.reg); \
        via_Value rhs_n = rget(rhs.reg); \
        VM_ASSERT(lhs_n.type == ValueType::Number, "Expected Number for binary operand 0"); \
        VM_ASSERT(rhs_n.type == ValueType::Number, "Expected Number for binary operand 1"); \
        rset(dst.reg, lhs_n.num + rhs_n.num); \
        VM_NEXT(); \
    }

    case OpCode::ADD:
        VM_BINOP(+=)
    case OpCode::SUB:
        VM_BINOP(-=)
    case OpCode::MUL:
        VM_BINOP(*=)
    case OpCode::DIV:
        VM_BINOP(/=)

    case OpCode::POW:
    {
        Operand dst = VM_OPND(0);
        Operand num = VM_OPND(1);
        via_Value* dst_p = rget_address(dst.reg);
        via_Value num_n = rget(num.reg);

        VM_ASSERT(num_n.type == ValueType::Number, "Expected Number for binary operand 0");

        dst_p->num = std::pow(dst_p->num, num_n.num);

        VM_NEXT();
    }

    case OpCode::NEG:
    {
        Operand dst = VM_OPND(0);
        Operand lhs = VM_OPND(1);
        via_Value lhs_n = rget(lhs.reg);

        VM_ASSERT(lhs_n.type == ValueType::Number, "Expected Number for binary operand 0");

        rset(dst.reg, via_Value(-lhs_n.num));

        VM_NEXT();
    }

#define VM_LOGICOP(op) \
    { \
        Operand dst = VM_OPND(0); \
        Operand lhs = VM_OPND(1); \
        Operand rhs = VM_OPND(2); \
        via_Value lhs_n = rget(lhs.reg); \
        via_Value rhs_n = rget(rhs.reg); \
        rset(dst.reg, via_Value(vtobool(lhs_n).boole op vtobool(rhs_n).boole)); \
        VM_NEXT(); \
    }

    case OpCode::BAND:
        VM_LOGICOP(&&);
    case OpCode::BOR:
        VM_LOGICOP(||);
    case OpCode::BXOR:
        VM_LOGICOP(!=);

    case OpCode::BNOT:
    {
        Operand dst = VM_OPND(0);
        Operand lhs = VM_OPND(1);
        via_Value lhs_n = rget(lhs.reg);

        VM_ASSERT(lhs_n.type == ValueType::Bool, "Expected Bool for logical operand 0");

        rset(dst.reg, via_Value(!lhs_n.boole));

        VM_NEXT();
    }

#define VM_CMPOP(fn) \
    { \
        Operand dst = VM_OPND(0); \
        Operand lhs = VM_OPND(1); \
        Operand rhs = VM_OPND(2); \
        rset(dst.reg, via_Value(fn(lhs.reg, rhs.reg))); \
        VM_NEXT(); \
    }

    case OpCode::EQ:
        VM_CMPOP(rcmp);
    case OpCode::NEQ:
        VM_CMPOP(!rcmp);
        /*
        case OpCode::LT:
            VM_BINOP(<);
        case OpCode::GT:
            VM_BINOP(>);
        case OpCode::LE:
            VM_BINOP(<=);
        case OpCode::GE:
            VM_BINOP(>=);
        */

    case OpCode::STDOUT:
    {
        Operand src_r = VM_OPND(0);
        via_Value src = rget(src_r.reg);

        std::cout << vtostring(src).str << "\n";

        VM_NEXT();
    }

    case OpCode::GCADD:
    {
        Operand addr_r = VM_OPND(0);
        via_Value addr_v = rget(addr_r.reg);

        VM_ASSERT(addr_v.type == ValueType::Ptr, "Expected Ptr for GCADD");

        m_gc.add(reinterpret_cast<void*>(addr_v.ptr));

        VM_NEXT();
    }

    case OpCode::GCCOL:
        m_gc.collect();
        VM_NEXT();

    case OpCode::HALT:
        set_exit_data(0, "VM halted by user");
        break;

    case OpCode::EXIT:
    {
        via_Number code = rget<via_Number>({RegisterType::ER, 0});
        set_exit_data(static_cast<int>(code), "VM exited by user");
        VM_EXIT();
    }

    case OpCode::JMP:
    {
        Operand offset = VM_OPND(0);

        VM_JMP(static_cast<int>(offset.num));
        VM_NEXT();
    }

    case OpCode::JNZ:
    case OpCode::JZ:
    {
        Operand cond_r = VM_OPND(0);
        Operand offset = VM_OPND(1);

        via_Value condv = rget(cond_r.reg);
        via_Value& cond = vtonumber(condv);
        int actual_offset = static_cast<int>(offset.num);

        if (ip->op == OpCode::JNZ ? (cond.num != 0) : (cond.num == 0))
            VM_JMP(actual_offset);

        VM_NEXT();
    }

    case OpCode::CALL:
    {
        Operand id = VM_OPND(0);

        if (m_stack.is_empty())
        {
            via_Value g_calling = gget(id.ident);
            call(g_calling);
        }
        else
        {
            via_Value l_calling = lget(id.ident);
            call(l_calling);
        }

        VM_NEXT();
    }

    case OpCode::RET:
    {
        ret();
        VM_NEXT();
    }

    case OpCode::LABEL:
    {
        Operand id = VM_OPND(0);

        m_labels[std::string_view(id.ident)] = reinterpret_cast<Instruction*>(ip - ihp);

        while (reinterpret_cast<Instruction*>(ip - ihp) < ibp)
        {
            if (ip->op == OpCode::END)
            {
                ip++;
                break;
            }
            ip++;
        }

        VM_DISPATCH();
    }

    case OpCode::FUNC:
    {
        Operand id = VM_OPND(0);

        if (lget(id.ident).type == ValueType::Nil)
        {
            Instruction* ip_copy = ip;
            lset(id.ident, via_Value(ip_copy));
            VM_NEXT();
        }

        while (reinterpret_cast<Instruction*>(ip - ihp) < ibp)
        {
            if (ip->op == OpCode::END)
            {
                ip++;
                break;
            }
            ip++;
        }

        VM_DISPATCH();
    }

    case OpCode::INSERT:
    {
        Operand tbl_r = VM_OPND(0);
        Operand val_r = VM_OPND(1);
        via_Value tbl = rget(tbl_r.reg);
        via_Value val = rget(val_r.reg);

        VM_ASSERT(tbl.type == ValueType::Table, "Attempt to insert into non-table value");

        tinsert(tbl.tbl, val);

        VM_NEXT();
    }

    case OpCode::CALLM:
    {
        Operand tbl_r = VM_OPND(0);
        via_Value tbl = rget(tbl_r.reg);
        via_Value key = rget(Register{.type = RegisterType::IR, .offset = 0});

        VM_ASSERT(tbl.type == ValueType::Table, "Attempt to index non-table type");
        VM_ASSERT(key.type == ValueType::TableKey, "Attempt to index table with non-key type");

        tcallm(tbl.tbl, *key.tblkey);

        VM_NEXT();
    }

    case OpCode::LOADIDX:
    {
        Operand tbl_r = VM_OPND(0);
        Operand dst_r = VM_OPND(1);
        via_Value tbl = rget(tbl_r.reg);
        via_Value key = rget(Register{.type = RegisterType::IR, .offset = 0});

        VM_ASSERT(tbl.type == ValueType::Table, "Attempt to load index of non-table type");
        VM_ASSERT(key.type == ValueType::TableKey, "Attempt to load table index of non-key type");

        tload(tbl.tbl, *key.tblkey, dst_r.reg);

        VM_NEXT();
    }

    case OpCode::SETIDX:
    {
        Operand tbl_r = VM_OPND(0);
        Operand src_r = VM_OPND(1);
        via_Value tbl = rget(tbl_r.reg);
        via_Value key = rget(Register{.type = RegisterType::IR, .offset = 0});

        VM_ASSERT(tbl.type == ValueType::Table, "Attempt to set index of non-table type");
        VM_ASSERT(key.type == ValueType::TableKey, "Attempt to set table index of non-key type");

        via_Table* t = tbl.tbl;
        t->set(*key.tblkey, rget(src_r.reg));

        VM_NEXT();
    }

    case OpCode::LEN:
    {
        Operand dst_r = VM_OPND(0);
        Operand obj_r = VM_OPND(1);

        rset(dst_r.reg, via_Value(len(rget(obj_r.reg))));

        VM_NEXT();
    }

    case OpCode::FREEZE:
    {
        Operand tbl_r = VM_OPND(0);
        via_Value tbl = rget(tbl_r.reg);

        VM_ASSERT(tbl.type == ValueType::Table, "Attempt to freeze non-table value");

        tfreeze(tbl.tbl);

        VM_NEXT();
    }

    case OpCode::ISFROZEN:
    {
        Operand dst_r = VM_OPND(0);
        Operand tbl_r = VM_OPND(1);
        via_Value tbl = rget(tbl_r.reg);

        VM_ASSERT(tbl.type == ValueType::Table, "Attempt to query isfrozen on non-table value");

        rset(dst_r.reg, via_Value(tisfrozen(tbl.tbl)));

        VM_NEXT();
    }

    case OpCode::TOSTRING:
    {
        Operand dst_r = VM_OPND(0);
        Operand val_r = VM_OPND(1);
        via_Value val = rget(val_r.reg);

        rset(dst_r.reg, vtostring(val));

        VM_NEXT();
    }

    case OpCode::TONUMBER:
    {
        Operand dst_r = VM_OPND(0);
        Operand val_r = VM_OPND(1);
        via_Value val = rget(val_r.reg);

        rset(dst_r.reg, vtonumber(val));

        VM_NEXT();
    }

    case OpCode::TOBOOL:
    {
        Operand dst_r = VM_OPND(0);
        Operand val_r = VM_OPND(1);
        via_Value val = rget(val_r.reg);

        rset(dst_r.reg, vtobool(val));

        VM_NEXT();
    }

    case OpCode::FSREAD:
    {
        Operand dst_r = VM_OPND(0);
        Operand path_r = VM_OPND(1);
        via_Value path_v = rget(path_r.reg);

        VM_ASSERT(path_v.type == ValueType::String, "Expected string for file path");

        auto path = std::filesystem::path(path_v.str);

        std::ostringstream buf;
        std::ofstream f(path);

        VM_ASSERT(f.is_open(), "Failed to open file");

        via_String fc = strdup(buf.str().c_str());
        gcadd(fc);
        rset(dst_r.reg, fc);

        f.close();

        VM_NEXT();
    }

    case OpCode::FSWRITE:
    {
        Operand src_r = VM_OPND(0);
        Operand path_r = VM_OPND(1);
        via_Value path_v = rget(path_r.reg);

        VM_ASSERT(path_v.type == ValueType::String, "Expected string for file path");

        auto path = std::filesystem::path(path_v.str);

        std::ostringstream buf;
        std::ofstream f(path);

        VM_ASSERT(f.is_open(), "Failed to open file");

        f << src_r.str;
        f.close();

        VM_NEXT();
    }

    case OpCode::FSMKDIR:
    {
        Operand path_r = VM_OPND(0);
        via_Value path_v = rget(path_r.reg);

        VM_ASSERT(path_v.type == ValueType::String, "Expected string for file path");

        auto path = std::filesystem::path(path_v.str);

        VM_ASSERT(!std::filesystem::exists(path), "Failed to make directory: path already exists");

        bool success = std::filesystem::create_directory(path);

        VM_ASSERT(success, "Failed to make directory");
        VM_NEXT();
    }

    case OpCode::FSRM:
    {
        Operand path_r = VM_OPND(0);
        via_Value path_v = rget(path_r.reg);

        VM_ASSERT(path_v.type == ValueType::String, "Expected string for file path");

        auto path = std::filesystem::path(path_v.str);

        VM_ASSERT(std::filesystem::exists(path), "Failed to remove directory: path DNE");

        bool success = std::filesystem::remove(path);

        VM_ASSERT(success, "Failed to remove directory");
        VM_NEXT();
    }

    case OpCode::TYPE:
    {
        Operand dst_r = VM_OPND(0);
        Operand obj_r = VM_OPND(1);

        rset(dst_r.reg, vtype(rget(obj_r.reg)));

        VM_NEXT();
    }

    case OpCode::TYPEOF:
    {
        Operand dst_r = VM_OPND(0);
        Operand obj_r = VM_OPND(1);

        rset(dst_r.reg, vtypeof(rget(obj_r.reg)));

        VM_NEXT();
    }

    case OpCode::ISNIL:
    {
        Operand dst_r = VM_OPND(0);
        Operand obj_r = VM_OPND(1);

        rset(dst_r.reg, via_Value(rget(obj_r.reg).type == ValueType::Nil));

        VM_NEXT();
    }

    case OpCode::STRCON:
    {
        Operand dst_r = VM_OPND(0);
        Operand lhs_r = VM_OPND(1);
        Operand rhs_r = VM_OPND(2);

        via_Value lhs = rget(lhs_r.reg);
        via_Value rhs = rget(rhs_r.reg);

        VM_ASSERT(lhs.type == ValueType::String, "Attempt to concatenate non-string value");
        VM_ASSERT(rhs.type == ValueType::String, "Attempt to concatenate string with non-string value");

        rset(dst_r.reg, via_Value(strcat(lhs.str, rhs.str)));

        VM_NEXT();
    }

    case OpCode::STRSUB:
    {
        Operand dst_r = VM_OPND(0);
        Operand src_r = VM_OPND(1);
        Operand i_r = VM_OPND(2);
        Operand j_r = VM_OPND(3);

        via_Value src = rget(src_r.reg);
        via_Value i = rget(i_r.reg);
        via_Value j = rget(j_r.reg);

        VM_ASSERT(src.type == ValueType::String, "Attempt to take substring of non-string value");

        VM_ASSERT(i.type == ValueType::Number, "Expected number for substring param i");
        VM_ASSERT(j.type == ValueType::Number, "Expected number for substring param j");

        std::string src_str(src.str);
        std::string sub_str = src_str.substr(static_cast<size_t>(i.num), static_cast<size_t>(j.num));
        via_String str = strdup(sub_str.c_str());

        gcadd(str);
        rset(dst_r.reg, via_Value(str));

        VM_NEXT();
    }

    case OpCode::STRUP:
    {
        Operand dst_r = VM_OPND(0);
        Operand src_r = VM_OPND(1);

        via_Value src = rget(src_r.reg);

        VM_ASSERT(src.type == ValueType::String, "Attempt to uppercase non-string value");

        std::string str(src.str);
        auto src_str = std::transform(
            str.begin(),
            str.end(),
            str.begin(),
            [](unsigned char c)
            {
                return std::toupper(c);
            }
        );

        // Idk wtf is going on here
        // But hey, clangd thinks it's fine!
        via_String str_cpy = strdup(src_str._Ptr);

        gcadd(str_cpy);
        rset(dst_r.reg, via_Value(str_cpy));

        VM_NEXT();
    }

    case OpCode::STRLOW:
    {
        Operand dst_r = VM_OPND(0);
        Operand src_r = VM_OPND(1);

        via_Value src = rget(src_r.reg);

        VM_ASSERT(src.type == ValueType::String, "Attempt to lowercase non-string value");

        std::string str(src.str);
        auto src_str = std::transform(
            str.begin(),
            str.end(),
            str.begin(),
            [](unsigned char c)
            {
                return std::tolower(c);
            }
        );
        via_String str_cpy = strdup(src_str._Ptr);

        gcadd(str_cpy);
        rset(dst_r.reg, via_Value(str_cpy));

        VM_NEXT();
    }

    case OpCode::ALLOC:
    {
        Operand addr_r = VM_OPND(0);
        Operand size_r = VM_OPND(1);
        via_Value size = rget(size_r.reg);

        VM_ASSERT(size.type == ValueType::Number, "Expected number for allocation size");

        void* mem = std::malloc(static_cast<size_t>(size.num));

        VM_ASSERT(mem != nullptr, "Failed to allocate memory");

        rset(addr_r.reg, via_Value(mem));

        VM_NEXT();
    }

    case OpCode::FREE:
    {
        Operand addr_r = VM_OPND(0);
        via_Value addr = rget(addr_r.reg);

        VM_ASSERT(addr.type == ValueType::Ptr, "Expected Ptr for FREE");

        uintptr_t actual_addr = static_cast<uintptr_t>(addr.ptr);

        VM_ASSERT(actual_addr != 0, "Attempt to free null pointer");

        std::free(reinterpret_cast<void*>(actual_addr));

        VM_NEXT();
    }

    case OpCode::MEMCPY:
    {
        Operand dst_addr_r = VM_OPND(0);
        Operand src_addr_r = VM_OPND(1);
        via_Value dst = rget(dst_addr_r.reg);
        via_Value src = rget(src_addr_r.reg);

        VM_ASSERT(dst.type == ValueType::Ptr, "Expected Ptr for MEMCPY operand 0");
        VM_ASSERT(src.type == ValueType::Ptr, "Expected Ptr for MEMCPY operand 1");

        memcpy(reinterpret_cast<void*>(dst.ptr), reinterpret_cast<void*>(src.ptr), sizeof(via_Value));

        VM_NEXT();
    }

    default:
        set_exit_data(1, std::format("Unrecognized OpCode (op_id={})", static_cast<uint8_t>(ip->op)));
        VM_EXIT();
    }
}

exit:
    return m_state.exit_code;
}

void VirtualMachine::init()
{
    VIA_ASSERT(!is_running(), "Attempt to initialize VM while running");

    try
    {
        m_state.is_running = true;
        execute();
    }
    catch (const std::exception& e)
    {
        std::cout << std::format(
            "\nVM exiting with code=1 and exit_message='Exception thrown during "
            "execution: {}'\n",
            e.what()
        );
        return;
    }

    std::cout << std::format("\nVM exiting with code={} and exit_message='{}'\n", m_state.exit_code, m_state.exit_message);

    return;
}