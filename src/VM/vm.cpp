#include "vm.h"

using namespace via::VM;

#define VM_POS() \
    reinterpret_cast<Instruction*>(ip - ip_s)

#define VM_EXIT() \
    { goto exit; }

#define VM_DISPATCH() \
    { goto dispatch; }

#define VM_LOAD() \
    do { \
        if (!is_valid_jump_address(ip + 1)) { \
            set_exit_data(0, ""); \
            VM_EXIT(); \
        } \
        ip++; \
    } while(0)

#define VM_NEXT() \
    do { \
        VM_LOAD(); \
        VM_DISPATCH(); \
    } while (0);

#define VM_OPND(off) \
    (ip->operandv[off])

#define VM_ASSERT_TYPE(t0, t1) \
    do { \
        if (t0 != t1) { \
            set_exit_data(1, std::format( \
                "VM type assertion failed (expected {}, got {})\n  in file {}, line {}", \
                magic_enum::enum_name(t0), magic_enum::enum_name(t1), \
                __FILE__, __LINE__).c_str()); \
            VM_EXIT(); \
        } \
    } while (0)

#define VM_ASSERT(cond, message) \
    do { \
        if (!(cond)) { \
            set_exit_data(1, std::format( \
                "VM_ASSERT(): {}\n in file {}, line {}", \
                (message), __FILE__, __LINE__).c_str()); \
            VM_EXIT(); \
        } \
    } while (0)

#define VM_JMPTO(to) \
    do { \
        jmpto(to); \
        VM_DISPATCH(); \
    } while (0)

#define VM_JMP(offset) \
    do { \
        jmp(offset); \
        VM_DISPATCH(); \
    } while (0);

int VirtualMachine::execute()
{
    VM_DISPATCH();

dispatch: {

    if (get_fflag("FFLAG_ABRT"))
    {
        VM_EXIT();
    }

    if (get_fflag("FFLAG_SKIP"))
    {
        set_fflag("FFLAG_SKIP", false);
        VM_NEXT();
    }

    VM_ASSERT(ip && ip <= ip_e && ip >= ip_s,
        std::format("Instruction pointer out of bounds (ip={}, ip_s={}, ip_e={})",
            reinterpret_cast<uintptr_t>(ip),
            reinterpret_cast<uintptr_t>(ip_s),
            reinterpret_cast<uintptr_t>(ip_e)
        )
    );

    switch (ip->op)
    {
    case OpCode::END:
    case OpCode::NOP:
        VM_NEXT();

    case OpCode::MOV:
    {
        Operand dst_r = VM_OPND(0);
        Operand src_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(src_r.type, Operand::OType::Register);

        rset(dst_r.reg, rget(src_r.reg));
        rset(dst_r.reg, via_Value());

        VM_NEXT();
    }

    case OpCode::LOAD:
    {
        Operand dst_r = VM_OPND(0);
        Operand src_p = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(src_p.type, Operand::OType::Number);

        /* TODO: Make this safe */
        rset(dst_r.reg, *reinterpret_cast<via_Value*>(static_cast<uintptr_t>(src_p.num)));

        VM_NEXT();
    }

    case OpCode::STORE:
    {
        Operand dst_p = VM_OPND(0);
        Operand src_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_p.type, Operand::OType::Number);
        VM_ASSERT_TYPE(src_r.type, Operand::OType::Register);

        /* TODO: Make this safe */
        *reinterpret_cast<via_Value*>(static_cast<uintptr_t>(dst_p.num)) = rget(src_r.reg);

        VM_NEXT();
    }

    case OpCode::LI:
    {
        Operand dst_r = VM_OPND(0);
        via_Value val = interpret_operand(VM_OPND(1));

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        
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

        VM_ASSERT_TYPE(id.type, Operand::OType::Identifier);

        lset(id.ident, interpret_operand(val));

        VM_NEXT();
    }

    case OpCode::GETLOCAL:
    {
        Operand id = VM_OPND(0);
        Operand dst = VM_OPND(1);

        VM_ASSERT_TYPE(id.type, Operand::OType::Identifier);
        VM_ASSERT_TYPE(dst.type, Operand::OType::Register);

        lload(id.ident, dst.reg);

        VM_NEXT();
    }

#define VM_BINOP(op) \
    { \
        Operand dst = VM_OPND(0); \
        Operand lhs = VM_OPND(1); \
        Operand rhs = VM_OPND(2); \
        VM_ASSERT_TYPE(dst.type, Operand::OType::Register); \
        VM_ASSERT_TYPE(lhs.type, Operand::OType::Register); \
        VM_ASSERT_TYPE(rhs.type, Operand::OType::Register); \
        via_Value lhs_n = rget(lhs.reg); \
        via_Value rhs_n = rget(rhs.reg); \
        VM_ASSERT_TYPE(lhs_n.type, via_Value::VType::Number); \
        VM_ASSERT_TYPE(rhs_n.type, via_Value::VType::Number); \
        via_Value res = via_Value(lhs_n.num op rhs_n.num); \
        rset(dst.reg, res); \
        VM_NEXT(); \
    }

    case OpCode::ADD: VM_BINOP(+)
    case OpCode::SUB: VM_BINOP(-)
    case OpCode::MUL: VM_BINOP(*)
    case OpCode::DIV: VM_BINOP(/)

    case OpCode::NEG:
    {
        Operand dst = VM_OPND(0);
        Operand lhs = VM_OPND(1);
        
        VM_ASSERT_TYPE(dst.type, Operand::OType::Register);
        VM_ASSERT_TYPE(lhs.type, Operand::OType::Register);

        via_Value lhs_n = rget(lhs.reg);

        VM_ASSERT_TYPE(lhs_n.type, via_Value::VType::Number);

        rset(dst.reg, via_Value(-lhs_n.num));

        VM_NEXT();
    }

#define VM_LOGICOP(op) \
    { \
        Operand dst = VM_OPND(0); \
        Operand lhs = VM_OPND(1); \
        Operand rhs = VM_OPND(2); \
        VM_ASSERT_TYPE(dst.type, Operand::OType::Register); \
        VM_ASSERT_TYPE(lhs.type, Operand::OType::Register); \
        VM_ASSERT_TYPE(rhs.type, Operand::OType::Register); \
        via_Value lhs_n = rget(lhs.reg); \
        via_Value rhs_n = rget(rhs.reg); \
        rset(dst.reg, via_Value(vtobool(lhs_n).boole op vtobool(rhs_n).boole)); \
        VM_NEXT(); \
    }

    case OpCode::AND: VM_LOGICOP(&&);
    case OpCode::OR:  VM_LOGICOP(||);
    case OpCode::XOR: VM_LOGICOP(!=);

    case OpCode::NOT:
    {
        Operand dst = VM_OPND(0);
        Operand lhs = VM_OPND(1);

        VM_ASSERT_TYPE(dst.type, Operand::OType::Register);
        VM_ASSERT_TYPE(lhs.type, Operand::OType::Register);

        via_Value lhs_n = rget(lhs.reg);

        VM_ASSERT_TYPE(lhs_n.type, via_Value::VType::Bool);

        rset(dst.reg, via_Value(!lhs_n.boole));

        VM_NEXT();
    }

#define VM_CMPOP(fn) \
    { \
        Operand dst = VM_OPND(0); \
        Operand lhs = VM_OPND(1); \
        Operand rhs = VM_OPND(2); \
        VM_ASSERT_TYPE(dst.type, Operand::OType::Register); \
        VM_ASSERT_TYPE(lhs.type, Operand::OType::Register); \
        VM_ASSERT_TYPE(rhs.type, Operand::OType::Register); \
        rset(dst.reg, via_Value(fn(lhs.reg, rhs.reg))); \
        VM_NEXT(); \
    }

    case OpCode::EQ:    VM_CMPOP(rcmp);
    case OpCode::NEQ:   VM_CMPOP(!rcmp);
    case OpCode::LT:    VM_BINOP(<);
    case OpCode::GT:    VM_BINOP(>);
    case OpCode::LE:    VM_BINOP(<=);
    case OpCode::GE:    VM_BINOP(>=);

    case OpCode::STDOUT:
    {
        Operand src_r = VM_OPND(0);

        VM_ASSERT_TYPE(src_r.type, Operand::OType::Register);

        std::cout << vtostring(rget(src_r.reg)).str << "\n";

        VM_NEXT();
    }

    case OpCode::GCADD:
    {
        Operand addr_r = VM_OPND(0);

        VM_ASSERT_TYPE(addr_r.type, Operand::OType::Register);

        via_Value addr_v = rget(addr_r.reg);

        VM_ASSERT_TYPE(addr_v.type, via_Value::VType::Ptr);

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
        Operand code_r = VM_OPND(0);

        VM_ASSERT_TYPE(code_r.type, Operand::OType::Register);

        via_Value code_v = rget(code_r.reg);

        VM_ASSERT(code_v.type == via_Value::VType::Number,
            "Attempt to exit with non-number exit code");
        
        set_exit_data(static_cast<int>(code_v.num), "VM exited by user");
        VM_EXIT();
    }

    case OpCode::JMP:
    {
        Operand offset = VM_OPND(0);

        VM_ASSERT_TYPE(offset.type, Operand::OType::Number);
        VM_JMP(static_cast<int>(offset.num));
        VM_NEXT();
    }

    case OpCode::JNZ:
    case OpCode::JZ:    
    {
        Operand cond_r = VM_OPND(0);
        Operand offset = VM_OPND(1);

        VM_ASSERT_TYPE(cond_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(offset.type, Operand::OType::Number);

        via_Value cond = vtobool(rget(cond_r.reg));
        int actual_offset = static_cast<int>(offset.num);

        if (ip->op == OpCode::JNZ ? cond.boole : !cond.boole)
            VM_JMP(actual_offset);
        
        VM_NEXT();
    }

    case OpCode::CALL:
    {
        Operand id = VM_OPND(0);

        VM_ASSERT_TYPE(id.type, Operand::OType::Identifier);

        via_Value calling = lget(id.ident);
        call(calling);

        VM_NEXT();
    }

    case OpCode::RET:
    {
        ret();
        VM_NEXT();
    }

    case OpCode::LABEL: {
        Operand id = VM_OPND(0);

        VM_ASSERT_TYPE(id.type, Operand::OType::Identifier);

        m_labels[std::string_view(id.ident)] = VM_POS();
        
        while (VM_POS() < ip_e)
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

    case OpCode::FUNC: {
        Operand id = VM_OPND(0);

        VM_ASSERT_TYPE(id.type, Operand::OType::Identifier);

        if (lget(id.ident).type == via_Value::VType::Nil)
        {
            Instruction* ip_copy = ip;
            lset(id.ident, via_Value(ip_copy));
            VM_NEXT();
        }

        while (VM_POS() < ip_e)
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

        VM_ASSERT_TYPE(tbl_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(val_r.type, Operand::OType::Register);

        via_Value tbl = rget(tbl_r.reg);
        via_Value val = rget(val_r.reg);

        VM_ASSERT(tbl.type == via_Value::VType::Table,
            "Attempt to insert into non-table value");

        tinsert(tbl.tbl, val);

        VM_NEXT();
    }

    case OpCode::CALLM:
    {
        Operand tbl_r = VM_OPND(0);

        VM_ASSERT_TYPE(tbl_r.type, Operand::OType::Register);

        via_Value tbl = rget(tbl_r.reg);
        via_Value key = rget(Register {
            .type = Register::RType::IR,
            .offset = 0
        });

        VM_ASSERT(tbl.type == via_Value::VType::Table, "Attempt to index non-table type");
        VM_ASSERT(key.type == via_Value::VType::TableKey,
            "Attempt to index table with non-key type");

        tcallm(tbl.tbl, *key.tblkey);

        VM_NEXT();
    }

    case OpCode::LOADIDX:
    {
        Operand tbl_r = VM_OPND(0);
        Operand dst_r = VM_OPND(1);

        VM_ASSERT_TYPE(tbl_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);

        via_Value tbl = rget(tbl_r.reg);
        via_Value key = rget(Register {
            .type = Register::RType::IR,
            .offset = 0
        });

        VM_ASSERT(tbl.type == via_Value::VType::Table, "Attempt to load index of non-table type");
        VM_ASSERT(key.type == via_Value::VType::TableKey,
            "Attempt to load table index of non-key type");

        tload(tbl.tbl, *key.tblkey, dst_r.reg);

        VM_NEXT();
    }

    case OpCode::SETIDX:
    {
        Operand tbl_r = VM_OPND(0);
        Operand src_r = VM_OPND(1);

        VM_ASSERT_TYPE(tbl_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(src_r.type, Operand::OType::Register);

        via_Value tbl = rget(tbl_r.reg);
        via_Value key = rget(Register{
            .type = Register::RType::IR,
            .offset = 0
        });

        VM_ASSERT(tbl.type == via_Value::VType::Table, "Attempt to set index of non-table type");
        VM_ASSERT(key.type == via_Value::VType::TableKey,
            "Attempt to set table index of non-key type");

        via_Table* t = tbl.tbl;
        t->set(*key.tblkey, rget(src_r.reg));

        VM_NEXT();
    }

    case OpCode::LEN:
    {
        Operand dst_r = VM_OPND(0);
        Operand obj_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(obj_r.type, Operand::OType::Register);

        rset(dst_r.reg, via_Value(len(rget(obj_r.reg))));

        VM_NEXT();
    }

    case OpCode::FREEZE:
    {
        Operand tbl_r = VM_OPND(0);

        VM_ASSERT_TYPE(tbl_r.type, Operand::OType::Register);

        via_Value tbl = rget(tbl_r.reg);

        VM_ASSERT(tbl.type == via_Value::VType::Table,
            "Attempt to freeze non-table value");

        tfreeze(tbl.tbl);

        VM_NEXT();
    }

    case OpCode::ISFROZEN:
    {
        Operand dst_r = VM_OPND(0);
        Operand tbl_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(tbl_r.type, Operand::OType::Register);

        via_Value tbl = rget(tbl_r.reg);

        VM_ASSERT(tbl.type == via_Value::VType::Table,
            "Attempt to query isfrozen on non-table value");

        rset(dst_r.reg, via_Value(tisfrozen(tbl.tbl)));

        VM_NEXT();
    }

    case OpCode::TOSTRING:
    {
        Operand dst_r = VM_OPND(0);
        Operand val_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(val_r.type, Operand::OType::Register);

        rset(dst_r.reg, via_Value(vtostring(rget(val_r.reg))));

        VM_NEXT();
    }

    case OpCode::TONUMBER:
    {
        Operand dst_r = VM_OPND(0);
        Operand val_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(val_r.type, Operand::OType::Register);

        rset(dst_r.reg, vtonumber(rget(val_r.reg)));

        VM_NEXT();
    }

    case OpCode::TOBOOL:
    {
        Operand dst_r = VM_OPND(0);
        Operand val_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(val_r.type, Operand::OType::Register);

        rset(dst_r.reg, via_Value(vtobool(rget(val_r.reg))));

        VM_NEXT();
    }

    case OpCode::FSREAD:
    {
        Operand dst_r = VM_OPND(0);
        Operand path_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(path_r.type, Operand::OType::Register);

        via_Value path_v = rget(path_r.reg);

        VM_ASSERT(path_v.type == via_Value::VType::String,
            "Expected string for file path");

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

        VM_ASSERT_TYPE(src_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(path_r.type, Operand::OType::Register);

        via_Value path_v = rget(path_r.reg);

        VM_ASSERT(path_v.type == via_Value::VType::String,
            "Expected string for file path");

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

        VM_ASSERT_TYPE(path_r.type, Operand::OType::Register);

        via_Value path_v = rget(path_r.reg);

        VM_ASSERT(path_v.type == via_Value::VType::String,
            "Expected string for file path");

        auto path = std::filesystem::path(path_v.str);

        VM_ASSERT(!std::filesystem::exists(path), "Failed to make directory: path already exists");

        bool success = std::filesystem::create_directory(path);

        VM_ASSERT(success, "Failed to make directory");
        VM_NEXT();
    }

    case OpCode::FSRM:
    {
        Operand path_r = VM_OPND(0);

        VM_ASSERT_TYPE(path_r.type, Operand::OType::Register);

        via_Value path_v = rget(path_r.reg);

        VM_ASSERT(path_v.type == via_Value::VType::String,
            "Expected string for file path");

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

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(obj_r.type, Operand::OType::Register);

        rset(dst_r.reg, vtype(rget(obj_r.reg)));

        VM_NEXT();
    }

    case OpCode::TYPEOF:
    {
        Operand dst_r = VM_OPND(0);
        Operand obj_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(obj_r.type, Operand::OType::Register);

        rset(dst_r.reg, vtypeof(rget(obj_r.reg)));

        VM_NEXT();
    }

    case OpCode::ISNIL:
    {
        Operand dst_r = VM_OPND(0);
        Operand obj_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(obj_r.type, Operand::OType::Register);

        rset(dst_r.reg, via_Value(rget(obj_r.reg).type == via_Value::VType::Nil));

        VM_NEXT();
    }

    case OpCode::STRCON:
    {
        Operand dst_r = VM_OPND(0);
        Operand lhs_r = VM_OPND(1);
        Operand rhs_r = VM_OPND(2);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(lhs_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(rhs_r.type, Operand::OType::Register);

        via_Value lhs = rget(lhs_r.reg);
        via_Value rhs = rget(rhs_r.reg);

        VM_ASSERT(lhs.type == via_Value::VType::String,
            "Attempt to concatenate non-string value");
        VM_ASSERT(rhs.type == via_Value::VType::String,
            "Attempt to concatenate string with non-string value");

        rset(dst_r.reg, via_Value(strcat(lhs.str, rhs.str)));

        VM_NEXT();
    }

    case OpCode::STRSUB:
    {
        Operand dst_r = VM_OPND(0);
        Operand src_r = VM_OPND(1);
        Operand i_r = VM_OPND(2);
        Operand j_r = VM_OPND(3);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(src_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(i_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(j_r.type, Operand::OType::Register);

        via_Value src = rget(src_r.reg);
        via_Value i = rget(i_r.reg);
        via_Value j = rget(j_r.reg);

        VM_ASSERT(src.type == via_Value::VType::String,
            "Attempt to take substring of non-string value");

        VM_ASSERT(i.type == via_Value::VType::Number, "Expected number for substring param i");
        VM_ASSERT(j.type == via_Value::VType::Number, "Expected number for substring param j");

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

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(src_r.type, Operand::OType::Register);

        via_Value src = rget(src_r.reg);

        VM_ASSERT(src.type == via_Value::VType::String, "Attempt to uppercase non-string value");

        std::string str(src.str);
        auto src_str = std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c) { return std::toupper(c); });
        via_String str_cpy = strdup(src_str.base());

        gcadd(str_cpy);
        rset(dst_r.reg, via_Value(str_cpy));
        
        VM_NEXT();
    }

    case OpCode::STRLOW:
    {
        Operand dst_r = VM_OPND(0);
        Operand src_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(src_r.type, Operand::OType::Register);

        via_Value src = rget(src_r.reg);

        VM_ASSERT(src.type == via_Value::VType::String, "Attempt to lowercase non-string value");

        std::string str(src.str);
        auto src_str = std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c) { return std::tolower(c); });
        via_String str_cpy = strdup(src_str.base());

        gcadd(str_cpy);
        rset(dst_r.reg, via_Value(str_cpy));

        VM_NEXT();
    }

    case OpCode::ALLOC:
    {
        Operand addr_r = VM_OPND(0);
        Operand size_r = VM_OPND(1);

        VM_ASSERT_TYPE(addr_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(size_r.type, Operand::OType::Register);

        via_Value size = rget(size_r.reg);

        VM_ASSERT(size.type == via_Value::VType::Number, "Expected number for allocation size");

        void* mem = std::malloc(static_cast<size_t>(size.num));

        VM_ASSERT(mem != nullptr, "Failed to allocate memory");

        rset(addr_r.reg, via_Value(mem));

        VM_NEXT();
    }

    case OpCode::FREE:
    {
        Operand addr_r = VM_OPND(0);

        VM_ASSERT_TYPE(addr_r.type, Operand::OType::Register);

        via_Value addr = rget(addr_r.reg);

        VM_ASSERT_TYPE(addr.type, via_Value::VType::Number);

        uintptr_t actual_addr = static_cast<uintptr_t>(addr.num);

        VM_ASSERT(actual_addr != 0, "Attempt to free null pointer");

        std::free(reinterpret_cast<void*>(actual_addr));

        VM_NEXT();
    }

    case OpCode::MEMCPY:
    {
        Operand dst_addr_r = VM_OPND(0);
        Operand src_addr_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_addr_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(src_addr_r.type, Operand::OType::Register);

        via_Value dst = rget(dst_addr_r.reg);
        via_Value src = rget(src_addr_r.reg);

        VM_ASSERT_TYPE(dst.type, via_Value::VType::Ptr);
        VM_ASSERT_TYPE(src.type, via_Value::VType::Ptr);

        memcpy(reinterpret_cast<void*>(dst.ptr),
            reinterpret_cast<void*>(src.ptr), sizeof(via_Value));

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

    m_state.is_running = true;

    try {
        execute();
    }
    catch (const std::exception& e) {
        std::cout << std::format("\nVM execution exception\n  {}\n", e.what());
        return;
    }

    std::cout << std::format("\nVM exiting with code={} and exit_message='{}'\n",
        m_state.exit_code, m_state.exit_message);

    return;
}