#include "vm.h"

using namespace via::VM;

#define VM_POS() \
    reinterpret_cast<Instruction*>(ip - iphead)

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

    VM_ASSERT(ip && ip <= ipend && ip >= iphead,
        std::format("Instruction pointer out of bounds (ip={}, iphead={}, ipend={})",
            reinterpret_cast<uintptr_t>(ip),
            reinterpret_cast<uintptr_t>(iphead),
            reinterpret_cast<uintptr_t>(ipend)
        )
    );

    switch (ip->op)
    {
    case OpCode::END:
    case OpCode::NOP:
        VM_NEXT();

    case OpCode::MOV:
    {
        auto dst_r = VM_OPND(0);
        auto src_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(src_r.type, Operand::OType::Register);

        rset(dst_r.reg_val, rget(src_r.reg_val));
        rset(dst_r.reg_val, via_Value());

        VM_NEXT();
    }

    case OpCode::LOAD:
    {
        auto dst_r = VM_OPND(0);
        auto src_p = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(src_p.type, Operand::OType::Number);

        /* TODO: Make this safe */
        rset(dst_r.reg_val, *reinterpret_cast<via_Value*>(static_cast<uintptr_t>(src_p.num_val)));

        VM_NEXT();
    }

    case OpCode::STORE:
    {
        auto dst_p = VM_OPND(0);
        auto src_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_p.type, Operand::OType::Number);
        VM_ASSERT_TYPE(src_r.type, Operand::OType::Register);

        /* TODO: Make this safe */
        *reinterpret_cast<via_Value*>(static_cast<uintptr_t>(dst_p.num_val)) = rget(src_r.reg_val);

        VM_NEXT();
    }

    case OpCode::LI:
    {
        auto dst_r = VM_OPND(0);
        auto val = interpret_operand(VM_OPND(1));

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        
        rset(dst_r.reg_val, val);

        VM_NEXT();
    }

    case OpCode::PUSH:
        stack.push();
        VM_NEXT();

    case OpCode::POP:
        stack.pop();
        VM_NEXT();

    case OpCode::SETLOCAL:
    {
        auto id = VM_OPND(0);
        auto val = VM_OPND(1);

        VM_ASSERT_TYPE(id.type, Operand::OType::Identifier);

        setl(id.ident_val, interpret_operand(val));

        VM_NEXT();
    }

    case OpCode::GETLOCAL:
    {
        auto id = VM_OPND(0);
        auto dst = VM_OPND(1);

        VM_ASSERT_TYPE(id.type, Operand::OType::Identifier);
        VM_ASSERT_TYPE(dst.type, Operand::OType::Register);

        loadl(id.ident_val, dst.reg_val);

        VM_NEXT();
    }

#define VM_BINOP(op) \
    { \
        auto dst = VM_OPND(0); \
        auto lhs = VM_OPND(1); \
        auto rhs = VM_OPND(2); \
        VM_ASSERT_TYPE(dst.type, Operand::OType::Register); \
        VM_ASSERT_TYPE(lhs.type, Operand::OType::Register); \
        VM_ASSERT_TYPE(rhs.type, Operand::OType::Register); \
        auto lhs_n = rget(lhs.reg_val); \
        auto rhs_n = rget(rhs.reg_val); \
        VM_ASSERT_TYPE(lhs_n.type, via_Value::VType::Number); \
        VM_ASSERT_TYPE(rhs_n.type, via_Value::VType::Number); \
        auto res = via_Value(lhs_n.num_val op rhs_n.num_val); \
        rset(dst.reg_val, res); \
        VM_NEXT(); \
    }

    case OpCode::ADD: VM_BINOP(+)
    case OpCode::SUB: VM_BINOP(-)
    case OpCode::MUL: VM_BINOP(*)
    case OpCode::DIV: VM_BINOP(/)

    case OpCode::NEG:
    {
        auto dst = VM_OPND(0);
        auto lhs = VM_OPND(1);
        
        VM_ASSERT_TYPE(dst.type, Operand::OType::Register);
        VM_ASSERT_TYPE(lhs.type, Operand::OType::Register);

        auto lhs_n = rget(lhs.reg_val);

        VM_ASSERT_TYPE(lhs_n.type, via_Value::VType::Number);

        rset(dst.reg_val, via_Value(-lhs_n.num_val));

        VM_NEXT();
    }

#define VM_LOGICOP(op) \
    { \
        auto dst = VM_OPND(0); \
        auto lhs = VM_OPND(1); \
        auto rhs = VM_OPND(2); \
        VM_ASSERT_TYPE(dst.type, Operand::OType::Register); \
        VM_ASSERT_TYPE(lhs.type, Operand::OType::Register); \
        VM_ASSERT_TYPE(rhs.type, Operand::OType::Register); \
        auto lhs_n = rget(lhs.reg_val); \
        auto rhs_n = rget(rhs.reg_val); \
        rset(dst.reg_val, via_Value(tobool(lhs_n) op tobool(rhs_n))); \
        VM_NEXT(); \
    }

    case OpCode::AND: VM_LOGICOP(&&);
    case OpCode::OR:  VM_LOGICOP(||);
    case OpCode::XOR: VM_LOGICOP(!=);

    case OpCode::NOT:
    {
        auto dst = VM_OPND(0);
        auto lhs = VM_OPND(1);

        VM_ASSERT_TYPE(dst.type, Operand::OType::Register);
        VM_ASSERT_TYPE(lhs.type, Operand::OType::Register);

        auto lhs_n = rget(lhs.reg_val);

        VM_ASSERT_TYPE(lhs_n.type, via_Value::VType::Bool);

        rset(dst.reg_val, via_Value(!lhs_n.bool_val));

        VM_NEXT();
    }

#define VM_CMPOP(fn) \
    { \
        auto dst = VM_OPND(0); \
        auto lhs = VM_OPND(1); \
        auto rhs = VM_OPND(2); \
        VM_ASSERT_TYPE(dst.type, Operand::OType::Register); \
        VM_ASSERT_TYPE(lhs.type, Operand::OType::Register); \
        VM_ASSERT_TYPE(rhs.type, Operand::OType::Register); \
        rset(dst.reg_val, via_Value(fn(lhs.reg_val, rhs.reg_val))); \
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
        auto src_r = VM_OPND(0);

        VM_ASSERT_TYPE(src_r.type, Operand::OType::Register);

        std::cout << tostring(rget(src_r.reg_val)) << "\n";

        VM_NEXT();
    }

    case OpCode::GCADD:
    {
        auto addr_r = VM_OPND(0);

        VM_ASSERT_TYPE(addr_r.type, Operand::OType::Register);

        auto addr_v = rget(addr_r.reg_val);

        VM_ASSERT_TYPE(addr_v.type, via_Value::VType::Ptr);

        gc.add(reinterpret_cast<void*>(addr_v.ptr_val));

        VM_NEXT();
    }

    case OpCode::GCCOL:
        gc.collect();
        VM_NEXT();

    case OpCode::HALT:
        set_exit_data(0, "VM halted by user");
        break;

    case OpCode::EXIT:
    {
        auto code_r = VM_OPND(0);

        VM_ASSERT_TYPE(code_r.type, Operand::OType::Register);

        auto code_v = rget(code_r.reg_val);

        VM_ASSERT(code_v.type == via_Value::VType::Number,
            "Attempt to exit with non-number exit code");
        
        set_exit_data(static_cast<int>(code_v.num_val), "VM exited by user");
        VM_EXIT();
    }

    case OpCode::JMP:
    {
        auto offset = VM_OPND(0);

        VM_ASSERT_TYPE(offset.type, Operand::OType::Number);
        VM_JMP(static_cast<int>(offset.num_val));
        VM_NEXT();
    }

    case OpCode::JNZ:
    case OpCode::JZ:    
    {
        auto cond_r = VM_OPND(0);
        auto offset = VM_OPND(1);

        VM_ASSERT_TYPE(cond_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(offset.type, Operand::OType::Number);

        auto cond = tobool(rget(cond_r.reg_val));
        auto actual_offset = static_cast<int>(offset.num_val);

        if (tobool(via_Value(ip->op == OpCode::JNZ ? cond : !cond)))
            VM_JMP(actual_offset);
        
        VM_NEXT();
    }

    case OpCode::CALL:
    {
        auto id = VM_OPND(0);

        VM_ASSERT_TYPE(id.type, Operand::OType::Identifier);

        auto frame = stack.top();
        auto calling = frame.get_local(id.ident_val);

        VM_ASSERT(calling.type == via_Value::VType::Func,
            "Attempt to call non-function value");

        call(*calling.fun_val);

        VM_NEXT();
    }

    case OpCode::RET:
    {
        ret();
        VM_NEXT();
    }

    case OpCode::LABEL: {
        auto id = VM_OPND(0);
        VM_ASSERT_TYPE(id.type, Operand::OType::Identifier);

        labels[std::string_view(id.ident_val)] = VM_POS();
        
        while (VM_POS() < ipend)
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
        auto id = VM_OPND(0);
        VM_ASSERT_TYPE(id.type, Operand::OType::Identifier);

        auto frame = stack.top();

        if (frame.get_local(id.ident_val).type == via_Value::VType::Nil)
        {
            Instruction* ip_copy = ip;
            setl(id.ident_val, via_Value(ip_copy));
            VM_NEXT();
        }

        while (VM_POS() < ipend)
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
        auto tbl_r = VM_OPND(0);
        auto val_r = VM_OPND(1);

        VM_ASSERT_TYPE(tbl_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(val_r.type, Operand::OType::Register);

        auto tbl = rget(tbl_r.reg_val);
        auto val = rget(val_r.reg_val);

        VM_ASSERT(tbl.type == via_Value::VType::Table,
            "Attempt to insert into non-table value");

        tinsert(tbl.tbl_val, val);

        VM_NEXT();
    }

    case OpCode::CALLM:
    {
        auto tbl_r = VM_OPND(0);

        VM_ASSERT_TYPE(tbl_r.type, Operand::OType::Register);

        auto tbl = rget(tbl_r.reg_val);
        auto key = rget(Register {
            .type = Register::RType::IR,
            .offset = 0
        });

        VM_ASSERT(tbl.type == via_Value::VType::Table, "Attempt to index non-table type");
        VM_ASSERT(key.type == via_Value::VType::TableKey,
            "Attempt to index table with non-key type");

        tcallm(tbl.tbl_val, *key.tblkey_val);

        VM_NEXT();
    }

    case OpCode::LOADIDX:
    {
        auto tbl_r = VM_OPND(0);
        auto dst_r = VM_OPND(1);

        VM_ASSERT_TYPE(tbl_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);

        auto tbl = rget(tbl_r.reg_val);
        auto key = rget(Register {
            .type = Register::RType::IR,
            .offset = 0
        });

        VM_ASSERT(tbl.type == via_Value::VType::Table, "Attempt to load index of non-table type");
        VM_ASSERT(key.type == via_Value::VType::TableKey,
            "Attempt to load table index of non-key type");

        tload(tbl.tbl_val, *key.tblkey_val, dst_r.reg_val);

        VM_NEXT();
    }

    case OpCode::SETIDX:
    {
        auto tbl_r = VM_OPND(0);
        auto src_r = VM_OPND(1);

        VM_ASSERT_TYPE(tbl_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(src_r.type, Operand::OType::Register);

        auto tbl = rget(tbl_r.reg_val);
        auto key = rget(Register{
            .type = Register::RType::IR,
            .offset = 0
        });

        VM_ASSERT(tbl.type == via_Value::VType::Table, "Attempt to set index of non-table type");
        VM_ASSERT(key.type == via_Value::VType::TableKey,
            "Attempt to set table index of non-key type");

        tset(tbl.tbl_val, *key.tblkey_val, rget(src_r.reg_val));

        VM_NEXT();
    }

    case OpCode::LEN:
    {
        auto dst_r = VM_OPND(0);
        auto obj_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(obj_r.type, Operand::OType::Register);

        rset(dst_r.reg_val, via_Value(len(rget(obj_r.reg_val))));

        VM_NEXT();
    }

    case OpCode::FREEZE:
    {
        auto tbl_r = VM_OPND(0);

        VM_ASSERT_TYPE(tbl_r.type, Operand::OType::Register);

        auto tbl = rget(tbl_r.reg_val);

        VM_ASSERT(tbl.type == via_Value::VType::Table,
            "Attempt to freeze non-table value");

        tfreeze(tbl.tbl_val);

        VM_NEXT();
    }

    case OpCode::ISFROZEN:
    {
        auto dst_r = VM_OPND(0);
        auto tbl_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(tbl_r.type, Operand::OType::Register);

        auto tbl = rget(tbl_r.reg_val);

        VM_ASSERT(tbl.type == via_Value::VType::Table,
            "Attempt to query isfrozen on non-table value");

        rset(dst_r.reg_val, via_Value(tisfrozen(tbl.tbl_val)));

        VM_NEXT();
    }

    case OpCode::TOSTRING:
    {
        auto dst_r = VM_OPND(0);
        auto val_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(val_r.type, Operand::OType::Register);

        rset(dst_r.reg_val, via_Value(tostring(rget(val_r.reg_val))));

        VM_NEXT();
    }

    case OpCode::TONUMBER:
    {
        auto dst_r = VM_OPND(0);
        auto val_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(val_r.type, Operand::OType::Register);

        auto nv = tonumber(rget(val_r.reg_val));
        auto n = std::visit([](const auto& v) {
            return via_Value(v);
        }, nv);

        rset(dst_r.reg_val, n);

        VM_NEXT();
    }

    case OpCode::TOBOOL:
    {
        auto dst_r = VM_OPND(0);
        auto val_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(val_r.type, Operand::OType::Register);

        rset(dst_r.reg_val, via_Value(tobool(rget(val_r.reg_val))));

        VM_NEXT();
    }

    case OpCode::FSREAD:
    {
        auto dst_r = VM_OPND(0);
        auto path_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(path_r.type, Operand::OType::Register);

        auto path_v = rget(path_r.reg_val);

        VM_ASSERT(path_v.type == via_Value::VType::String,
            "Expected string for file path");

        auto path = std::filesystem::path(path_v.str_val);

        std::ostringstream buf;
        std::ofstream f(path);
        
        VM_ASSERT(f.is_open(), "Failed to open file");

        auto fc = strdup(buf.str().c_str());
        gcadd(fc);
        rset(dst_r.reg_val, fc);

        f.close();

        VM_NEXT();
    }

    case OpCode::FSWRITE:
    {
        auto src_r = VM_OPND(0);
        auto path_r = VM_OPND(1);

        VM_ASSERT_TYPE(src_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(path_r.type, Operand::OType::Register);

        auto path_v = rget(path_r.reg_val);

        VM_ASSERT(path_v.type == via_Value::VType::String,
            "Expected string for file path");

        auto path = std::filesystem::path(path_v.str_val);

        std::ostringstream buf;
        std::ofstream f(path);

        VM_ASSERT(f.is_open(), "Failed to open file");

        f << src_r.str_val;
        f.close();

        VM_NEXT();
    }

    case OpCode::FSMKDIR:
    {
        auto path_r = VM_OPND(0);

        VM_ASSERT_TYPE(path_r.type, Operand::OType::Register);

        auto path_v = rget(path_r.reg_val);

        VM_ASSERT(path_v.type == via_Value::VType::String,
            "Expected string for file path");

        auto path = std::filesystem::path(path_v.str_val);

        VM_ASSERT(!std::filesystem::exists(path), "Failed to make directory: path already exists");

        bool success = std::filesystem::create_directory(path);

        VM_ASSERT(success, "Failed to make directory");
        VM_NEXT();
    }

    case OpCode::FSRM:
    {
        auto path_r = VM_OPND(0);

        VM_ASSERT_TYPE(path_r.type, Operand::OType::Register);

        auto path_v = rget(path_r.reg_val);

        VM_ASSERT(path_v.type == via_Value::VType::String,
            "Expected string for file path");

        auto path = std::filesystem::path(path_v.str_val);

        VM_ASSERT(std::filesystem::exists(path), "Failed to remove directory: path DNE");

        bool success = std::filesystem::remove(path);

        VM_ASSERT(success, "Failed to remove directory");
        VM_NEXT();
    }

    case OpCode::TYPE:
    {
        auto dst_r = VM_OPND(0);
        auto obj_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(obj_r.type, Operand::OType::Register);

        rset(dst_r.reg_val, vtype(rget(obj_r.reg_val)));

        VM_NEXT();
    }

    case OpCode::TYPEOF:
    {
        auto dst_r = VM_OPND(0);
        auto obj_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(obj_r.type, Operand::OType::Register);

        rset(dst_r.reg_val, vtypeof(rget(obj_r.reg_val)));

        VM_NEXT();
    }

    case OpCode::ISNIL:
    {
        auto dst_r = VM_OPND(0);
        auto obj_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(obj_r.type, Operand::OType::Register);

        rset(dst_r.reg_val, via_Value(rget(obj_r.reg_val).type == via_Value::VType::Nil));

        VM_NEXT();
    }

    case OpCode::STRCON:
    {
        auto dst_r = VM_OPND(0);
        auto lhs_r = VM_OPND(1);
        auto rhs_r = VM_OPND(2);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(lhs_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(rhs_r.type, Operand::OType::Register);

        auto lhs = rget(lhs_r.reg_val);
        auto rhs = rget(rhs_r.reg_val);

        VM_ASSERT(lhs.type == via_Value::VType::String,
            "Attempt to concatenate non-string value");
        VM_ASSERT(rhs.type == via_Value::VType::String,
            "Attempt to concatenate string with non-string value");

        rset(dst_r.reg_val, via_Value(strcat(lhs.str_val, rhs.str_val)));

        VM_NEXT();
    }

    case OpCode::STRSUB:
    {
        auto dst_r = VM_OPND(0);
        auto src_r = VM_OPND(1);
        auto i_r = VM_OPND(2);
        auto j_r = VM_OPND(3);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(src_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(i_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(j_r.type, Operand::OType::Register);

        auto src = rget(src_r.reg_val);
        auto i = rget(i_r.reg_val);
        auto j = rget(j_r.reg_val);

        VM_ASSERT(src.type == via_Value::VType::String,
            "Attempt to take substring of non-string value");

        VM_ASSERT(i.type == via_Value::VType::Number, "Expected number for substring param i");
        VM_ASSERT(j.type == via_Value::VType::Number, "Expected number for substring param j");

        auto src_str = std::string(src.str_val);
        auto sub_str = src_str.substr(static_cast<size_t>(i.num_val), static_cast<size_t>(j.num_val));
        auto str = strdup(sub_str.c_str());
        
        gcadd(str);
        rset(dst_r.reg_val, via_Value(str));

        VM_NEXT();
    }

    case OpCode::STRUP:
    {
        auto dst_r = VM_OPND(0);
        auto src_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(src_r.type, Operand::OType::Register);

        auto src = rget(src_r.reg_val);

        VM_ASSERT(src.type == via_Value::VType::String, "Attempt to uppercase non-string value");

        auto str = std::string(src.str_val);
        auto src_str = std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c) { return std::toupper(c); });
        auto str_cpy = strdup(src_str.base());

        gcadd(str_cpy);
        rset(dst_r.reg_val, via_Value(str_cpy));
        
        VM_NEXT();
    }

    case OpCode::STRLOW:
    {
        auto dst_r = VM_OPND(0);
        auto src_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(src_r.type, Operand::OType::Register);

        auto src = rget(src_r.reg_val);

        VM_ASSERT(src.type == via_Value::VType::String, "Attempt to lowercase non-string value");

        auto str = std::string(src.str_val);
        auto src_str = std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c) { return std::tolower(c); });
        auto str_cpy = strdup(src_str.base());

        gcadd(str_cpy);
        rset(dst_r.reg_val, via_Value(str_cpy));

        VM_NEXT();
    }

    case OpCode::ALLOC:
    {
        auto addr_r = VM_OPND(0);
        auto size_r = VM_OPND(1);

        VM_ASSERT_TYPE(addr_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(size_r.type, Operand::OType::Register);

        auto size = rget(size_r.reg_val);

        VM_ASSERT(size.type == via_Value::VType::Number, "Expected number for allocation size");

        auto mem = std::malloc(static_cast<size_t>(size.num_val));

        VM_ASSERT(mem != nullptr, "Failed to allocate memory");

        rset(addr_r.reg_val, via_Value(mem));

        VM_NEXT();
    }

    case OpCode::FREE:
    {
        auto addr_r = VM_OPND(0);

        VM_ASSERT_TYPE(addr_r.type, Operand::OType::Register);

        auto addr = rget(addr_r.reg_val);

        VM_ASSERT_TYPE(addr.type, via_Value::VType::Number);

        auto actual_addr = static_cast<uintptr_t>(addr.num_val);

        VM_ASSERT(actual_addr != 0, "Attempt to free null pointer");

        std::free(reinterpret_cast<void*>(actual_addr));

        VM_NEXT();
    }

    case OpCode::MEMCPY:
    {
        auto dst_addr_r = VM_OPND(0);
        auto src_addr_r = VM_OPND(1);

        VM_ASSERT_TYPE(dst_addr_r.type, Operand::OType::Register);
        VM_ASSERT_TYPE(src_addr_r.type, Operand::OType::Register);

        auto dst = rget(dst_addr_r.reg_val);
        auto src = rget(src_addr_r.reg_val);

        VM_ASSERT_TYPE(dst.type, via_Value::VType::Ptr);
        VM_ASSERT_TYPE(src.type, via_Value::VType::Ptr);

        memcpy(reinterpret_cast<void*>(dst.ptr_val),
            reinterpret_cast<void*>(src.ptr_val), sizeof(via_Value));

        VM_NEXT();
    }

    default:
        set_exit_data(1, std::format("Unrecognized OpCode (op_id={})", static_cast<uint8_t>(ip->op)));
        VM_EXIT();
    }
}

exit:
    return state.exit_code;
}

void VirtualMachine::init()
{
    VIA_ASSERT(!is_running(), "Attempt to initialize VM while running");

    state.is_running = true;

    try { execute(); }
    catch (const std::exception& e) {
        std::cout << std::format("\nVM execution exception\n  {}\n", e.what());
        return;
    }

    std::cout << std::format("\nVM exiting with code={} and exit_message='{}'\n",
        state.exit_code, state.exit_message);

    return;
}