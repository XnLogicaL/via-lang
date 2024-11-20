/* This file is a part of the via programming language at
 * https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

/*
 * !! WARNING !!
 * Unit testing is not allowed in this file!
 * This is due to the performance critical nature of it's contents
 */

#include "execute.h"
#include "api.h"
#include "bytecode.h"
#include "core.h"
#include "state.h"
#include "types.h"
#include <chrono>
#include <cstddef>

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
        if (!via_validjmpaddr(V, V->ip + 1)) \
        { \
            via_setexitdata(V, 0, ""); \
            VM_EXIT(); \
        } \
        V->ip++; \
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
#define VM_OPND(off) (V->ip->operandv[off])

// Standard VM assertion
// Terminates VM if the assertion fails
// Contains debug information, such as file, line and a custom message
#define VM_ASSERT(cond, message) \
    do \
    { \
        if (!(cond)) \
        { \
            via_setexitdata(V, 1, std::format("VM_ASSERT(): {}\n in file {}, line {}", (message), __FILE__, __LINE__).c_str()); \
            VM_EXIT(); \
        } \
    } while (0)

// Macro for jumping to a jump address
// Immediately dispatches the VM
#define VM_JMPTO(to) \
    do \
    { \
        via_jmpto(to); \
        VM_DISPATCH(); \
    } while (0)

// Macro for jumping to an offset relative to the current instruction pointer
// position Immediately dispatches VM
#define VM_JMP(offset) \
    do \
    { \
        via_jmpto(V, V->ip + offset); \
        VM_DISPATCH(); \
    } while (0);

#define VM_GETIDXREG() (via_getregister<viaTableKey>(V, {RegisterType::IR, 0}))
#define VM_GETSELFREG() (via_getregister<viaValue>(V, {RegisterType::SR, 0}))

namespace via
{

// This does not need to be inline
// Becaue it should only be called once
void via_execute(viaState *V)
{
    VIA_ASSERT(V->ts != viaThreadState::RUNNING, "via_execute() called on thread with status RUNNING");
    VIA_ASSERT(V->ts != viaThreadState::DEAD, "via_execute() called on dead thread");

    V->ts = viaThreadState::RUNNING;

    // Not necessary, but provides clarity
    VM_DISPATCH();

dispatch:
{
    // Check if the state needs to be restored
    if (VIA_UNLIKELY(V->restorestate) && VIA_LIKELY(V->sstate))
    {
        V->restorestate = false;
        via_restorestate(V);
    }

    // This is unlikely because it only occurs once
    if (VIA_UNLIKELY(V->abrt))
        VM_EXIT();

    // This is unlikely because most instructions don't invoke skip
    if (VIA_UNLIKELY(V->skip))
    {
        V->skip = false;
        VM_NEXT();
    }

    VM_ASSERT(
        // This is unlikely because for the ip to go out of bounds, the user specifically has to jump to an invalid address
        // Otherwise, this is completely impossible
        VIA_UNLIKELY(via_validjmpaddr(V, V->ip)),
        std::format(
            "viaInstruction pointer out of bounds (ip={}, ihp={},ibp={})",
            reinterpret_cast<uintptr_t>(V->ip),
            reinterpret_cast<uintptr_t>(V->ihp),
            reinterpret_cast<uintptr_t>(V->ibp)
        )
    );

    // This is unlikely because the VM very rarely yields at all
    if (VIA_UNLIKELY(V->yield))
    {
        long long waitt = static_cast<long long>(V->yieldfor * 1000);
        std::chrono::milliseconds dur = std::chrono::milliseconds(waitt);
        std::this_thread::sleep_for(dur);
        V->yield = false;
    }

    // No LOAD protocol is present here
    // This is because the LOAD protocol is invoked when VM_NEXT is called
    switch (V->ip->op)
    {
    case OpCode::END:
    case OpCode::NOP:
        VM_NEXT();

    case OpCode::MOV:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand rsrc = VM_OPND(1);

        // Fast-path: both operands are registers
        if (VIA_LIKELY(rdst.type == viaOperandType::viaRegister))
        {
            via_setregister(V, rdst.reg, via_getregister(V, rsrc.reg));
            via_setregister(V, rdst.reg, viaValue());
        }
        else
            via_setregister(V, rdst.reg, via_toviavalue(V, rsrc));

        VM_NEXT();
    }

    case OpCode::CPY:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand rsrc = VM_OPND(1);

        via_setregister(V, rdst.reg, via_getregister(V, rsrc.reg));

        VM_NEXT();
    }

    case OpCode::LOAD:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand psrc = VM_OPND(1);
        /* TODO: Make this safe */
        via_setregister(V, rdst.reg, *reinterpret_cast<viaValue *>(static_cast<uintptr_t>(psrc.num)));

        VM_NEXT();
    }

    case OpCode::STORE:
    {
        viaOperand pdst = VM_OPND(0);
        viaOperand rsrc = VM_OPND(1);
        /* TODO: Make this safe */
        *reinterpret_cast<viaValue *>(static_cast<uintptr_t>(pdst.num)) = *via_getregister(V, rsrc.reg);

        VM_NEXT();
    }

    case OpCode::LI:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand imm = VM_OPND(1);

        viaValue val = via_toviavalue(V, imm);
        via_setregister(V, rdst.reg, val);

        VM_NEXT();
    }

    case OpCode::NIL:
    {
        viaOperand rdst = VM_OPND(0);
        via_setregister(V, rdst.reg, viaValue());

        VM_NEXT();
    }

    case OpCode::PUSH:
    {
        viaInstruction *ipc = V->ip;
        V->stack->push(StackFrame(ipc, *V->gc));
        VM_NEXT();
    }

    case OpCode::POP:
    {
        V->stack->pop();
        VM_NEXT();
    }

    case OpCode::SETLOCAL:
    {
        viaOperand id = VM_OPND(0);
        viaOperand val = VM_OPND(1);

        via_setlocal(V, id.ident, via_toviavalue(V, val));

        VM_NEXT();
    }

    case OpCode::LOADLOCAL:
    {
        viaOperand id = VM_OPND(0);
        viaOperand dst = VM_OPND(1);

        via_loadlocal(V, id.ident, dst.reg);

        VM_NEXT();
    }

    case OpCode::SETGLOBAL:
    {
        viaOperand id = VM_OPND(0);
        viaOperand val = VM_OPND(1);

        via_setglobal(V, id.ident, via_toviavalue(V, val));

        VM_NEXT();
    }

    case OpCode::LOADGLOBAL:
    {
        viaOperand id = VM_OPND(0);
        viaOperand dst = VM_OPND(1);

        via_loadglobal(V, id.ident, dst.reg);

        VM_NEXT();
    }

    case OpCode::LOADVAR:
    {
        viaOperand id = VM_OPND(0);
        viaOperand dst = VM_OPND(1);

        viaValue local = via_getlocal(V, id.ident);

        if (local.type != viaValueType::Nil)
        {
            via_setregister(V, dst.reg, local);
            VM_NEXT();
        }

        // TODO: Implement stack unwinding to search for the variable

        viaValue global = via_getglobal(V, id.ident);
        via_setregister(V, dst.reg, global);

        VM_NEXT();
    }

#define VM_BINOP(op) \
    { \
        viaOperand dst = VM_OPND(0); \
        viaOperand lhs = VM_OPND(1); \
        viaOperand rhs = VM_OPND(2); \
        viaValue lhsn, rhsn; \
        bool is_lhs_number = false; \
        bool is_rhs_number = false; \
        if (VIA_LIKELY(lhs.type == viaOperandType::viaNumber)) \
        { \
            lhsn.num = lhs.num; \
            is_lhs_number = true; \
        } \
        else \
            lhsn = *via_getregister(V, lhs.reg); \
        if (VIA_LIKELY(rhs.type == viaOperandType::viaNumber)) \
        { \
            rhsn.num = rhs.num; \
            is_rhs_number = true; \
        } \
        else \
            rhsn = *via_getregister(V, rhs.reg); \
        if (is_lhs_number && is_rhs_number) \
            via_setregister(V, dst.reg, lhsn.num op rhsn.num); \
        else \
        { \
            VM_ASSERT(lhsn.type == viaValueType::viaNumber, "Expected viaNumber for binary operand 0"); \
            VM_ASSERT(rhsn.type == viaValueType::viaNumber, "Expected viaNumber for binary operand 1"); \
            via_setregister(V, dst.reg, lhsn.num op rhsn.num); \
        } \
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
        viaOperand dst = VM_OPND(0);
        viaOperand num = VM_OPND(1);

        viaValue *pdst = via_getregister(V, dst.reg);
        viaValue numn;

        // Fast-path: num is a constant number
        if (VIA_LIKELY(num.type == viaOperandType::viaNumber))
            numn.num = num.num;
        else
            // Exponent is in register, fetch it
            numn = *via_getregister(V, num.reg);

        VM_ASSERT(numn.type == viaValueType::viaNumber, "Expected viaNumber for binary operand 0");

        // Perform the power operation
        pdst->num = std::pow(pdst->num, numn.num);

        VM_NEXT();
    }

    case OpCode::NEG:
    {
        viaOperand dst = VM_OPND(0);
        viaOperand lhs = VM_OPND(1);

        viaValue lhsn = *via_getregister(V, lhs.reg);

        VM_ASSERT(lhsn.type == viaValueType::viaNumber, "Expected viaNumber for binary operand 0");

        via_setregister(V, dst.reg, viaValue(-lhsn.num));

        VM_NEXT();
    }

#define VM_LOGICOP(op) \
    { \
        viaOperand dst = VM_OPND(0); \
        viaOperand lhs = VM_OPND(1); \
        viaOperand rhs = VM_OPND(2); \
        viaValue lhsn, rhsn; \
        /* Check if lhs is a boolean or a register */ \
        if (VIA_LIKELY(lhs.type == viaOperandType::Bool)) \
            lhsn.boole = lhs.boole; \
        else \
            lhsn = *via_getregister(V, lhs.reg); \
        /* Check if rhs is a boolean or a register */ \
        if (VIA_LIKELY(rhs.type == viaOperandType::Bool)) \
            rhsn.boole = rhs.boole; \
        else \
            rhsn = *via_getregister(V, rhs.reg); \
        /* Perform the logical operation directly if both operands are booleans */ \
        via_setregister(V, dst.reg, viaValue(via_tobool(V, lhsn).boole op via_tobool(V, rhsn).boole)); \
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
        viaOperand dst = VM_OPND(0);
        viaOperand lhs = VM_OPND(1);
        viaValue lhsn = *via_getregister(V, lhs.reg);

        VM_ASSERT(lhsn.type == viaValueType::Bool, "Expected Bool for logical operand 0");

        via_setregister(V, dst.reg, viaValue(!lhsn.boole));

        VM_NEXT();
    }

#define VM_CMPOP(fn, fn2) \
    { \
        viaOperand dst = VM_OPND(0); \
        viaOperand lhs = VM_OPND(1); \
        viaOperand rhs = VM_OPND(2); \
        viaValue lhsn, rhsn; \
        bool lhs_reg = false; \
        bool rhs_reg = false; \
        if (VIA_UNLIKELY(lhs.type == viaOperandType::viaRegister)) \
        { \
            lhsn = *via_getregister(V, lhs.reg); \
            lhs_reg = true; \
        } \
        else \
            lhsn = via_toviavalue(V, lhs); \
        if (VIA_UNLIKELY(rhs.type == viaOperandType::viaRegister)) \
        { \
            rhsn = *via_getregister(V, rhs.reg); \
            rhs_reg = true; \
        } \
        else \
            rhsn = via_toviavalue(V, rhs); \
        if (lhs_reg && rhs_reg) \
            via_setregister(V, dst.reg, viaValue(fn(V, lhs.reg, rhs.reg))); \
        else if (lhs_reg) \
            via_setregister(V, dst.reg, viaValue(fn2(V, *via_getregister(V, lhs.reg), rhsn))); \
        else if (rhs_reg) \
            via_setregister(V, dst.reg, viaValue(fn2(V, *via_getregister(V, rhs.reg), lhsn))); \
        else \
            via_setregister(V, dst.reg, viaValue(fn2(V, lhsn, rhsn))); \
        VM_NEXT(); \
    }

    case OpCode::EQ:
        VM_CMPOP(via_cmpregister, via_compare);
    case OpCode::NEQ:
        VM_CMPOP(!via_cmpregister, !via_compare);
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
        viaOperand rsrc = VM_OPND(0);

        viaValue *src = via_getregister(V, rsrc.reg);
        std::cout << via_tostring(V, *src).str;

        VM_NEXT();
    }

    case OpCode::GCADD:
    {
        viaOperand raddr = VM_OPND(0);
        viaValue addrv = *via_getregister(V, raddr.reg);

        VM_ASSERT(addrv.type == viaValueType::Ptr, "Expected Ptr for GCADD");
        via_gcadd(V, reinterpret_cast<void *>(addrv.ptr));

        VM_NEXT();
    }

    case OpCode::GCCOL:
    {
        via_gccol(V);
        VM_NEXT();
    }

    case OpCode::HALT:
    {
        via_setexitdata(V, 0, "VM halted by user");
        VM_EXIT();
    }

    case OpCode::EXIT:
    {
        viaNumber *code = via_getregister<viaNumber>(V, {RegisterType::ER, 0});
        via_setexitdata(V, static_cast<int>(*code), "VM exited by user");
        VM_EXIT();
    }

    case OpCode::JMP:
    {
        viaOperand offset = VM_OPND(0);

        VM_JMP(static_cast<int>(offset.num));
        VM_NEXT();
    }

    case OpCode::JNZ:
    case OpCode::JZ:
    {
        viaOperand condr = VM_OPND(0);
        viaOperand offset = VM_OPND(1);

        viaValue condv = *via_getregister(V, condr.reg);
        viaValue &cond = via_tonumber(V, condv);

        int actual_offset = static_cast<int>(offset.num);
        if (V->ip->op == OpCode::JNZ ? (cond.num != 0) : (cond.num == 0))
            VM_JMP(actual_offset);

        VM_NEXT();
    }

    case OpCode::CALL:
    {
        viaOperand id = VM_OPND(0);

        // Slow-path: the called function is located outside the current stack frame
        if (VIA_UNLIKELY(V->stack->is_empty()))
        {
            viaValue g_calling = via_getglobal(V, id.ident);
            via_call(V, g_calling);
        }
        else
        {
            viaValue l_calling = via_getlocal(V, id.ident);
            via_call(V, l_calling);
        }

        VM_NEXT();
    }

    case OpCode::RET:
    {
        if (V->stack->is_empty())
        {
            via_setexitdata(V, 1, "Callstack underflow");
            V->abrt = true;
            return;
        }

        // Return address
        auto ra = V->stack->top().retaddr;
        V->stack->pop();

        if (!via_validjmpaddr(V, const_cast<viaInstruction *>(ra)))
        {
            via_setexitdata(V, 1, "invalid return address");
            V->abrt = true;
            return;
        }

        // Flush argument registers
        V->ralloc.flush(RegisterType::AR);
        V->ralloc.flush(RegisterType::SR);

        // Jump back to the stack return address
        via_jmpto(V, const_cast<viaInstruction *>(ra) + 1);
        VM_NEXT();
    }

    case OpCode::LABEL:
    {
        viaOperand id = VM_OPND(0);

        (*V->labels)[std::string_view(id.ident)] = reinterpret_cast<viaInstruction *>(V->ip - V->ihp);

        while (reinterpret_cast<viaInstruction *>(V->ip - V->ihp) < V->ibp)
        {
            if (V->ip->op == OpCode::END)
            {
                V->ip++;
                break;
            }

            V->ip++;
        }

        VM_DISPATCH();
    }

    case OpCode::FUNC:
    {
        viaOperand id = VM_OPND(0);

        if (via_getlocal(V, id.ident).type == viaValueType::Nil)
        {
            via_setlocal(V, id.ident, viaValue(V->ip));
            VM_NEXT();
        }

        while (reinterpret_cast<viaInstruction *>(V->ip - V->ihp) < V->ibp)
        {
            if (V->ip->op == OpCode::END)
            {
                V->ip++;
                break;
            }

            V->ip++;
        }

        VM_DISPATCH();
    }

    case OpCode::INSERT:
    {
        viaOperand tblr = VM_OPND(0);
        viaOperand valr = VM_OPND(1);

        viaValue tbl = *via_getregister(V, tblr.reg);
        viaValue val;

        // Slow-path: inserted value is a register
        if (VIA_UNLIKELY(valr.type == viaOperandType::viaRegister))
            val = *via_getregister(V, valr.reg);
        else
            val = via_toviavalue(V, valr);

        VM_ASSERT(tbl.type == viaValueType::viaTable, "Attempt to insert into non-table value");
        via_inserttable(V, tbl.tbl, val);

        VM_NEXT();
    }

    case OpCode::CALLM:
    {
        viaOperand tblr = VM_OPND(0);
        viaValue tbl = *via_getregister(V, tblr.reg);
        // No, this is not unsafe
        // The IR register must hold a viaTableKey; therefore this is type safe
        viaTableKey key = *VM_GETIDXREG();

        VM_ASSERT(tbl.type == viaValueType::viaTable, "Attempt to index non-table type");
        via_callmethod(V, tbl.tbl, key);

        VM_NEXT();
    }

    case OpCode::LOADIDX:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand rtbl = VM_OPND(1);

        viaValue tbl = *via_getregister(V, rtbl.reg);
        viaTableKey key = *VM_GETIDXREG();

        VM_ASSERT(tbl.type == viaValueType::viaTable, "Attempt to load index of non-table type");

        via_loadtableindex(V, tbl.tbl, key, rdst.reg);

        VM_NEXT();
    }

    case OpCode::SETIDX:
    {
        viaOperand tblr = VM_OPND(0);
        viaOperand rsrc = VM_OPND(1);

        viaValue tbl = *via_getregister(V, tblr.reg);
        viaTableKey key = *VM_GETIDXREG();
        viaValue val;

        VM_ASSERT(tbl.type == viaValueType::viaTable, "Attempt to set index of non-table type");

        // Slow-path: the value is stored in a register
        if (VIA_UNLIKELY(rsrc.type == viaOperandType::viaRegister))
            val = *via_getregister(V, rsrc.reg);
        else
            val = via_toviavalue(V, rsrc);

        viaTable *T = tbl.tbl;
        via_settableindex(V, T, key, val);

        VM_NEXT();
    }

    case OpCode::LEN:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand objr = VM_OPND(1);

        viaValue val;

        // Fast-path: object is stored in a register
        if (VIA_LIKELY(objr.type == viaOperandType::viaRegister))
            val = *via_getregister(V, objr.reg);
        else
            val = via_toviavalue(V, objr);

        via_setregister(V, rdst.reg, via_len(V, val));

        VM_NEXT();
    }

    case OpCode::FREEZE:
    {
        viaOperand tblr = VM_OPND(0);

        viaValue T = *via_getregister(V, tblr.reg);
        VM_ASSERT(T.type == viaValueType::viaTable, "Attempt to freeze non-table value");
        via_freeze(V, T.tbl);

        VM_NEXT();
    }

    case OpCode::ISFROZEN:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand tblr = VM_OPND(1);

        viaValue tbl = *via_getregister(V, tblr.reg);
        VM_ASSERT(tbl.type == viaValueType::viaTable, "Attempt to query isfrozen on non-table value");
        via_setregister(V, rdst.reg, viaValue(via_isfrozen(V, tbl.tbl)));

        VM_NEXT();
    }

    case OpCode::TOSTRING:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand valr = VM_OPND(1);

        viaValue val = *via_getregister(V, valr.reg);

        via_setregister(V, rdst.reg, val);

        VM_NEXT();
    }

    case OpCode::TONUMBER:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand valr = VM_OPND(1);

        viaValue val = *via_getregister(V, valr.reg);

        via_setregister(V, rdst.reg, via_tonumber(V, val));

        VM_NEXT();
    }

    case OpCode::TOBOOL:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand valr = VM_OPND(1);

        viaValue val = *via_getregister(V, valr.reg);
        via_setregister(V, rdst.reg, via_tobool(V, val));

        VM_NEXT();
    }

    case OpCode::TYPE:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand objr = VM_OPND(1);

        via_setregister(V, rdst.reg, via_type(V, *via_getregister(V, objr.reg)));

        VM_NEXT();
    }

    case OpCode::TYPEOF:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand objr = VM_OPND(1);

        via_setregister(V, rdst.reg, via_typeof(V, *via_getregister(V, objr.reg)));

        VM_NEXT();
    }

    case OpCode::ISNIL:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand objr = VM_OPND(1);

        via_setregister(V, rdst.reg, viaValue(via_getregister(V, objr.reg)->type == viaValueType::Nil));

        VM_NEXT();
    }

    case OpCode::STRCON:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand lhsr = VM_OPND(1);
        viaOperand rhsr = VM_OPND(2);

        viaValue lhs = *via_getregister(V, lhsr.reg);
        viaValue rhs = *via_getregister(V, rhsr.reg);

        VM_ASSERT(lhs.type == viaValueType::String, "Attempt to concatenate non-string value");
        VM_ASSERT(rhs.type == viaValueType::String, "Attempt to concatenate string with non-string value");

        via_setregister(V, rdst.reg, viaValue(strcat(lhs.str, rhs.str)));

        VM_NEXT();
    }

    default:
        via_setexitdata(V, 1, std::format("Unrecognized OpCode (op_id={})", static_cast<uint8_t>(V->ip->op)));
        VM_EXIT();
    }
}

exit:
    return;
}

void via_killthread(viaState *V)
{
    if (V->ts == viaThreadState::RUNNING)
        // TODO: Wait for the VM to exit
        V->abrt = true;

    // Mark as dead thread
    V->ts = viaThreadState::DEAD;
    // Destroy state object (thread)
    delete V;
    // Decrement the thread_id to make room for more threads (I know you can technically make 4 billion threads ok?)
    __thread_id--;
}

void via_pausethread(viaState *V)
{
    V->ts = viaThreadState::PAUSED;
    // Save the VM state to restore it when paused
    via_savestate(V);
}

} // namespace via