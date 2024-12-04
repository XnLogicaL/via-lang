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
#include "instruction.h"
#include "shared.h"
#include "state.h"
#include "types.h"
#include <chrono>
#include <cmath>
#include <cstdint>

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
    { \
        if (!via_validjmpaddr(V, V->ip + 1)) \
        { \
            via_setexitdata(V, 0, ""); \
            VM_EXIT(); \
        } \
        V->ip++; \
    }

// Macro that "signals" the VM has completed an execution cycle
#define VM_NEXT() \
    { \
        VM_LOAD(); \
        VM_DISPATCH(); \
    }

// Returns the operand at offset <off>
// ! Can cause undefined behavior
#define VM_OPND(off) (V->ip->operandv[off])

// Standard VM assertion
// Terminates VM if the assertion fails
// Contains debug information, such as file, line and a custom message
#define VM_ASSERT(cond, message) \
    { \
        if (!(cond)) \
        { \
            via_setexitdata(V, 1, std::format("VM_ASSERT(): {}\n in file {}, line {}", (message), __FILE__, __LINE__).c_str()); \
            VM_EXIT(); \
        } \
    }

// Macro for jumping to a jump address
// Immediately dispatches the VM
#define VM_JMPTO(to) \
    { \
        via_jmpto(V, to); \
        VM_DISPATCH(); \
    }

// Macro for jumping to an offset relative to the current instruction pointer
// position Immediately dispatches VM
#define VM_JMP(offset) \
    { \
        via_jmpto(V, V->ip + offset); \
        VM_DISPATCH(); \
    }

namespace via
{

inline bool _is_empty_instruction(viaInstruction *instr)
{
    return instr->op == OpCode::NOP || instr->op == OpCode::END;
}

// Internal function that optimizes a sequence of empty instructions (such as NOP or END)
// By replacing the first instruction with a jmp instruction that jumps over the sequence
inline void _optimize_empty_instruction_sequence(viaState *V)
{
    // Check for an empty instruction at the current IP
    if (VIA_UNLIKELY(_is_empty_instruction(V->ip)))
    {
        viaJmpOffset_t skip_count = 1;

        // Find the end of the sequence of empty instructions
        while (V->ip + skip_count < V->ibp && _is_empty_instruction(V->ip + skip_count))
            skip_count++;

        // If there are multiple empty instructions, replace the first one with a JMP
        if (skip_count > 1)
        {
            V->ip->op = OpCode::JMP;
            V->ip->operandv[0] = viaOperand{.type = viaOperandType_t::Number, .val_number = static_cast<double>(skip_count)};
        }
    }
}

// This does not need to be inline
// Becaue it should only be called once
void via_execute(viaState *V)
{
    VIA_ASSERT(V->tstate != viaThreadState::RUNNING, "via_execute() called on thread with status RUNNING");
    VIA_ASSERT(V->tstate != viaThreadState::DEAD, "via_execute() called on dead thread");

    V->tstate = viaThreadState::RUNNING;

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
            "viaInstruction pointer out of bounds (ip={}, ihp={}, ibp={})",
            reinterpret_cast<const void *>(V->ip),
            reinterpret_cast<const void *>(V->ihp),
            reinterpret_cast<const void *>(V->ibp)
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
    case OpCode::ERR:
    {
        via_setexitdata(V, 1, "ERR OpCode");
        VM_EXIT();
    }

    case OpCode::END:
    case OpCode::NOP:
    {
        // Attempt to optimize empty instruction sequence
        _optimize_empty_instruction_sequence(V);
        VM_NEXT();
    }

    case OpCode::MOV:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand rsrc = VM_OPND(1);

        // Fast-path: both operands are registers
        if (VIA_LIKELY(rdst.type == viaOperandType_t::Register))
        {
            via_setregister(V, rdst.val_register, *via_getregister(V, rsrc.val_register));
            // Set the <src> register to nil
            via_setregister(V, rdst.val_register, viaT_stackvalue(V));
        }
        else
            // Basically just LI lmao
            via_setregister(V, rdst.val_register, via_toviavalue(V, rsrc));

        VM_NEXT();
    }

    // Basically MOV but the registers aren't cleaned
    case OpCode::CPY:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand rsrc = VM_OPND(1);

        // This has to be copied, otherwise it will remain a reference
        viaValue cpy = *via_getregister(V, rsrc.val_register);
        via_setregister(V, rdst.val_register, cpy);

        VM_NEXT();
    }

    case OpCode::LI:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand imm = VM_OPND(1);

        viaValue val = via_toviavalue(V, imm);
        via_setregister(V, rdst.val_register, val);

        VM_NEXT();
    }

    case OpCode::NIL:
    {
        viaOperand rdst = VM_OPND(0);
        // Set register to nil
        via_setregister(V, rdst.val_register, viaT_stackvalue(V));

        VM_NEXT();
    }

    case OpCode::SETLOCAL:
    {
        viaOperand id = VM_OPND(0);
        viaOperand val = VM_OPND(1);

        viaVariableIdentifier_t id_t = id.val_number;

        // Slow-path: loaded value is a register
        if (VIA_UNLIKELY(val.type == viaOperandType_t::Register))
            via_setvariable(V, id_t, *via_getregister(V, val.val_register));
        else
            via_setvariable(V, id_t, via_toviavalue(V, val));

        VM_NEXT();
    }

    case OpCode::LOADLOCAL:
    {
        viaOperand id = VM_OPND(0);
        viaOperand dst = VM_OPND(1);

        viaVariableIdentifier_t id_t = id.val_number;

        via_loadvariable(V, id_t, dst.val_register);
        VM_NEXT();
    }

    case OpCode::SETGLOBAL:
    {
        viaOperand id = VM_OPND(0);
        viaOperand val = VM_OPND(1);

        // Slow-path: loaded value is a register
        if (VIA_UNLIKELY(val.type == viaOperandType_t::Register))
            via_setglobal(V, viaT_hashstring(V, id.val_identifier), *via_getregister(V, val.val_register));
        else
            via_setglobal(V, viaT_hashstring(V, id.val_identifier), via_toviavalue(V, val));

        VM_NEXT();
    }

    case OpCode::LOADGLOBAL:
    {
        viaOperand id = VM_OPND(0);
        viaOperand dst = VM_OPND(1);

        via_loadglobal(V, viaT_hashstring(V, id.val_identifier), dst.val_register);

        VM_NEXT();
    }

    case OpCode::LOADVAR:
    {
        viaOperand id = VM_OPND(0);
        viaOperand dst = VM_OPND(1);

        viaVariableIdentifier_t id_t = id.val_number;
        viaValue local = via_getvariable(V, id_t);

        if (!viaT_checknil(V, local))
        {
            via_setregister(V, dst.val_register, local);
            VM_NEXT();
        }

        // TODO: Implement stack unwinding to search for the variable

        viaValue *global = via_getglobal(V, viaT_hashstring(V, id.val_identifier));
        via_setregister(V, dst.val_register, *global);

        VM_NEXT();
    }

#define VM_BINOP(op) \
    { \
        viaOperand dst = VM_OPND(0); \
        viaOperand lhs = VM_OPND(1); \
        viaOperand rhs = VM_OPND(2); \
        viaValue lhsn, rhsn; \
        if (VIA_LIKELY(lhs.type == viaOperandType_t::Number)) \
            lhsn.val_number = lhs.val_number; \
        else \
            lhsn = *via_getregister(V, lhs.val_register); \
        if (VIA_LIKELY(rhs.type == viaOperandType_t::Number)) \
            rhsn.val_number = rhs.val_number; \
        else \
            rhsn = *via_getregister(V, rhs.val_register); \
        via_setregister(V, dst.val_register, viaT_stackvalue(V, lhsn.val_number op rhsn.val_number)); \
        VM_NEXT(); \
    }

#define VM_IBINOP(op) \
    { \
        viaOperand dst = VM_OPND(0); \
        viaOperand lhs = VM_OPND(1); \
        viaValue lhsn; \
        viaValue *dstn = via_getregister(V, dst.val_register); \
        if (VIA_LIKELY(lhs.type == viaOperandType_t::Number)) \
            lhsn.val_number = lhs.val_number; \
        else \
            lhsn = *via_getregister(V, lhs.val_register); \
        dstn->val_number op lhsn.val_number; \
        VM_NEXT(); \
    }

    case OpCode::ADD:
        VM_BINOP(+)
    case OpCode::SUB:
        VM_BINOP(-)
    case OpCode::MUL:
        VM_BINOP(*)
    case OpCode::DIV:
        VM_BINOP(/)
    case OpCode::IADD:
        VM_IBINOP(+=);
    case OpCode::ISUB:
        VM_IBINOP(-=);
    case OpCode::IMUL:
        VM_IBINOP(*=);
    case OpCode::IDIV:
        VM_IBINOP(/=);

    case OpCode::IPOW:
    {
        viaOperand dst = VM_OPND(0);
        viaOperand num = VM_OPND(1);

        viaValue *pdst = via_getregister(V, dst.val_register);
        viaValue numn;

        // Fast-path: num is a constant number
        if (VIA_LIKELY(num.type == viaOperandType_t::Number))
            numn.val_number = num.val_number;
        else
            // Exponent is in register, fetch it
            numn = *via_getregister(V, num.val_register);

        // Perform the power operation
        pdst->val_number = std::pow(pdst->val_number, numn.val_number);

        VM_NEXT();
    }

    case OpCode::NEG:
    {
        viaOperand dst = VM_OPND(0);
        viaOperand lhs = VM_OPND(1);

        viaValue lhsn = *via_getregister(V, lhs.val_register);

        VM_ASSERT(viaT_checknumber(V, lhsn), "Expected Number for binary operand 0");

        via_setregister(V, dst.val_register, viaT_stackvalue(V, -lhsn.val_number));

        VM_NEXT();
    }

#define VM_LOGICOP(op) \
    { \
        viaOperand dst = VM_OPND(0); \
        viaOperand lhs = VM_OPND(1); \
        viaOperand rhs = VM_OPND(2); \
        viaValue lhsn{}, rhsn{}; \
        /* Check if lhs is a boolean or a register */ \
        if (VIA_LIKELY(lhs.type == viaOperandType_t::Bool)) \
            lhsn.val_boolean = lhs.val_boolean; \
        else \
            lhsn = *via_getregister(V, lhs.val_register); \
        /* Check if rhs is a boolean or a register */ \
        if (VIA_LIKELY(rhs.type == viaOperandType_t::Bool)) \
            rhsn.val_boolean = rhs.val_boolean; \
        else \
            rhsn = *via_getregister(V, rhs.val_register); \
        /* Perform the logical operation directly if both operands are booleans */ \
        via_setregister(V, dst.val_register, viaT_stackvalue(V, via_tobool(V, lhsn).val_boolean op via_tobool(V, rhsn).val_boolean)); \
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
        viaValue lhsn = *via_getregister(V, lhs.val_register);

        VM_ASSERT(viaT_checkbool(V, lhsn), "Expected Bool for logical operand 0");

        via_setregister(V, dst.val_register, viaT_stackvalue(V, !lhsn.val_boolean));

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
        if (VIA_UNLIKELY(lhs.type == viaOperandType_t::Register)) \
        { \
            lhsn = *via_getregister(V, lhs.val_register); \
            lhs_reg = true; \
        } \
        else \
            lhsn = via_toviavalue(V, lhs); \
        if (VIA_UNLIKELY(rhs.type == viaOperandType_t::Register)) \
        { \
            rhsn = *via_getregister(V, rhs.val_register); \
            rhs_reg = true; \
        } \
        else \
            rhsn = via_toviavalue(V, rhs); \
        if (lhs_reg && rhs_reg) \
            via_setregister(V, dst.val_register, viaT_stackvalue(V, fn(V, lhs.val_register, rhs.val_register))); \
        else if (lhs_reg) \
            via_setregister(V, dst.val_register, viaT_stackvalue(V, fn2(V, *via_getregister(V, lhs.val_register), rhsn))); \
        else if (rhs_reg) \
            via_setregister(V, dst.val_register, viaT_stackvalue(V, fn2(V, *via_getregister(V, rhs.val_register), lhsn))); \
        else \
            via_setregister(V, dst.val_register, viaT_stackvalue(V, fn2(V, lhsn, rhsn))); \
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

        viaValue *src = via_getregister(V, rsrc.val_register);
        std::cout << via_tostring(V, *src).val_string->ptr;

        VM_NEXT();
    }

    case OpCode::GCADD:
    {
        viaOperand raddr = VM_OPND(0);
        viaValue addrv = *via_getregister(V, raddr.val_register);

        VM_ASSERT(viaT_checkptr(V, addrv), "Expected Ptr for GCADD");
        via_gcadd(V, reinterpret_cast<void *>(addrv.val_pointer));

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
        viaOperand rcode = VM_OPND(0);

        viaValue *code = via_getregister(V, rcode.val_register);
        viaNumber ecode = via_tonumber(V, *code).val_number;
        via_setexitdata(V, static_cast<viaExitCode_t>(ecode), "VM exited by user");

        VM_EXIT();
    }

    case OpCode::JMP:
    {
        viaOperand offset = VM_OPND(0);

        VM_JMP(static_cast<viaJmpOffset_t>(offset.val_number));
        VM_NEXT();
    }

    case OpCode::JNZ:
    case OpCode::JZ:
    {
        viaOperand condr = VM_OPND(0);
        viaOperand offset = VM_OPND(1);

        viaValue cond = *via_getregister(V, condr.val_register);
        // We don't need to save the return value because this function modifies `val`
        via_tonumber(V, cond);

        viaJmpOffset_t actual_offset = static_cast<viaJmpOffset_t>(offset.val_number);
        if (V->ip->op == OpCode::JNZ ? (cond.val_number != 0) : (cond.val_number == 0))
            VM_JMP(actual_offset);

        VM_NEXT();
    }

    case OpCode::JEQ:
    case OpCode::JNEQ:
    {
        viaOperand condlr = VM_OPND(0);
        viaOperand condrr = VM_OPND(1);
        viaOperand offset = VM_OPND(2);

        bool cond = via_cmpregister(V, condlr.val_register, condrr.val_register);

        viaJmpOffset_t actual_offset = static_cast<viaJmpOffset_t>(offset.val_number);
        if (V->ip->op == OpCode::JEQ ? cond : !cond)
            VM_JMP(actual_offset);

        VM_NEXT();
    }

    case OpCode::JLT:
    case OpCode::JNLT:
    {
        viaOperand condlr = VM_OPND(0);
        viaOperand condrr = VM_OPND(1);
        viaOperand offset = VM_OPND(2);

        bool cond = via_getregister(V, condlr.val_register)->val_number < via_getregister(V, condrr.val_register)->val_number;

        viaJmpOffset_t actual_offset = static_cast<viaJmpOffset_t>(offset.val_number);
        if (V->ip->op == OpCode::JEQ ? cond : !cond)
            VM_JMP(actual_offset);

        VM_NEXT();
    }

    case OpCode::JGT:
    case OpCode::JNGT:
    {
        viaOperand condlr = VM_OPND(0);
        viaOperand condrr = VM_OPND(1);
        viaOperand offset = VM_OPND(2);

        bool cond = via_getregister(V, condlr.val_register)->val_number > via_getregister(V, condrr.val_register)->val_number;

        viaJmpOffset_t actual_offset = static_cast<viaJmpOffset_t>(offset.val_number);
        if (V->ip->op == OpCode::JEQ ? cond : !cond)
            VM_JMP(actual_offset);

        VM_NEXT();
    }

    case OpCode::JLE:
    case OpCode::JNLE:
    {
        viaOperand condlr = VM_OPND(0);
        viaOperand condrr = VM_OPND(1);
        viaOperand offset = VM_OPND(2);

        bool cond = via_getregister(V, condlr.val_register)->val_number <= via_getregister(V, condrr.val_register)->val_number;

        viaJmpOffset_t actual_offset = static_cast<viaJmpOffset_t>(offset.val_number);
        if (V->ip->op == OpCode::JEQ ? cond : !cond)
            VM_JMP(actual_offset);

        VM_NEXT();
    }

    case OpCode::JGE:
    case OpCode::JNGE:
    {
        viaOperand condlr = VM_OPND(0);
        viaOperand condrr = VM_OPND(1);
        viaOperand offset = VM_OPND(2);

        bool cond = via_getregister(V, condlr.val_register)->val_number >= via_getregister(V, condrr.val_register)->val_number;

        viaJmpOffset_t actual_offset = static_cast<viaJmpOffset_t>(offset.val_number);
        if (V->ip->op == OpCode::JEQ ? cond : !cond)
            VM_JMP(actual_offset);

        VM_NEXT();
    }

    case OpCode::JL:
    {
        viaOperand label = VM_OPND(0);

        auto it = V->labels->find(std::string_view(label.val_identifier));

        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));
        VM_JMPTO(it->second);
        VM_NEXT();
    }

    case OpCode::JLZ:
    case OpCode::JLNZ:
    {
        viaOperand valr = VM_OPND(0);
        viaOperand label = VM_OPND(1);

        auto it = V->labels->find(std::string_view(label.val_identifier));

        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));

        viaValue *val = via_getregister(V, valr.val_register);
        bool cond = val->val_number == 0;

        // Jump if the condition is met
        if (V->ip->op == OpCode::JLZ ? cond : !cond)
            VM_JMPTO(it->second);

        VM_NEXT();
    }

    case OpCode::JLEQ:
    case OpCode::JLNEQ:
    {
        viaOperand lhsr = VM_OPND(0);
        viaOperand rhsr = VM_OPND(1);
        viaOperand label = VM_OPND(2);

        auto it = V->labels->find(std::string_view(label.val_identifier));

        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));

        bool cond = via_cmpregister(V, lhsr.val_register, rhsr.val_register);

        // Jump if the condition is met
        if (V->ip->op == OpCode::JLEQ ? cond : !cond)
            VM_JMPTO(it->second);

        VM_NEXT();
    }

    case OpCode::JLLT:
    case OpCode::JLNLT:
    {
        viaOperand lhsr = VM_OPND(0);
        viaOperand rhsr = VM_OPND(1);
        viaOperand label = VM_OPND(2);

        auto it = V->labels->find(std::string_view(label.val_identifier));

        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));

        bool cond = via_getregister(V, lhsr.val_register)->val_number < via_getregister(V, rhsr.val_register)->val_number;

        // Jump if the condition is met
        if (V->ip->op == OpCode::JLLT ? cond : !cond)
            VM_JMPTO(it->second);

        VM_NEXT();
    }

    case OpCode::JLGT:
    case OpCode::JLNGT:
    {
        viaOperand lhsr = VM_OPND(0);
        viaOperand rhsr = VM_OPND(1);
        viaOperand label = VM_OPND(2);

        auto it = V->labels->find(std::string_view(label.val_identifier));

        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));

        bool cond = via_getregister(V, lhsr.val_register)->val_number > via_getregister(V, rhsr.val_register)->val_number;

        // Jump if the condition is met
        if (V->ip->op == OpCode::JLLT ? cond : !cond)
            VM_JMPTO(it->second);

        VM_NEXT();
    }

    case OpCode::JLLE:
    case OpCode::JLNLE:
    {
        viaOperand lhsr = VM_OPND(0);
        viaOperand rhsr = VM_OPND(1);
        viaOperand label = VM_OPND(2);

        auto it = V->labels->find(std::string_view(label.val_identifier));

        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));

        bool cond = via_getregister(V, lhsr.val_register)->val_number <= via_getregister(V, rhsr.val_register)->val_number;

        // Jump if the condition is met
        if (V->ip->op == OpCode::JLLT ? cond : !cond)
            VM_JMPTO(it->second);

        VM_NEXT();
    }

    case OpCode::JLGE:
    case OpCode::JLNGE:
    {
        viaOperand lhsr = VM_OPND(0);
        viaOperand rhsr = VM_OPND(1);
        viaOperand label = VM_OPND(2);

        auto it = V->labels->find(std::string_view(label.val_identifier));

        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));

        bool cond = via_getregister(V, lhsr.val_register)->val_number >= via_getregister(V, rhsr.val_register)->val_number;

        // Jump if the condition is met
        if (V->ip->op == OpCode::JLLT ? cond : !cond)
            VM_JMPTO(it->second);

        VM_NEXT();
    }

    case OpCode::CALL:
    {
        viaOperand rfn = VM_OPND(0);
        viaOperand argc = VM_OPND(1);

        // Set argc
        V->argc = static_cast<viaCallArgC_t>(argc.val_number);
        // Call function
        via_call(V, *via_getregister(V, rfn.val_register));

        VM_NEXT();
    }

    case OpCode::RET:
    {
        viaS_pop(V->stack);
        via_restorestate(V);

        VM_NEXT();
    }

    case OpCode::LABEL:
    {
        viaOperand id = VM_OPND(0);
        viaInstruction *addr = reinterpret_cast<viaInstruction *>(V->ip - V->ihp);

        (*V->labels)[std::string_view(id.val_identifier)] = addr;

        while (addr < V->ibp)
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
        viaOperand rfn = VM_OPND(0);
        viaFunction *fn = new viaFunction;
        fn->line = 0;
        // TODO: Accept a valid user-set ID
        fn->id = "<anonymous-function>";
        fn->consts = {};
        fn->locals = {};

        via_setregister(V, rfn.val_register, viaT_stackvalue(V, fn));

        // Directly compare instruction pointer with instruction base pointer
        // We don't need a temporary variable
        while (V->ip < V->ibp)
        {
            if (V->ip->op == OpCode::END)
            {
                // Mark OpCode as NOP so the VM skips this instruction
                V->ip->op = OpCode::NOP;
                V->ip++;
                break;
            }

            // Copy the instruction and insert into the function object
            viaInstruction cpy = *V->ip;
            fn->bytecode.push_back(cpy);

            // Mark OpCode as NOP so the VM skips this instruction
            V->ip->op = OpCode::NOP;
            V->ip++;
        }

        // Dispatch instead of invoking VM_NEXT
        VM_DISPATCH();
    }

    case OpCode::CALLM:
    {
        viaOperand rtbl = VM_OPND(0);
        viaOperand ridx = VM_OPND(1);

        viaValue tbl = *via_getregister(V, rtbl.val_register);
        viaValue idx = *via_getregister(V, ridx.val_register);

        // Get table key based on the index type (string or number)
        viaTableKey key = viaT_checkstring(V, idx) ? idx.val_string->hash : static_cast<viaHash_t>(idx.val_number);

        // Assert that the value is a table
        std::string _errfmt = std::format("Attempt to index {} with '{}'", via_type(V, tbl).val_string->ptr, key);
        VM_ASSERT(viaT_checktable(V, tbl), _errfmt);

        // Call the method from the table
        via_callmethod(V, tbl.val_table, key);
        VM_NEXT();
    }

    case OpCode::LOADIDX:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand rtbl = VM_OPND(1);
        viaOperand ridx = VM_OPND(2);

        viaValue tbl = *via_getregister(V, rtbl.val_register);
        viaValue idx = *via_getregister(V, ridx.val_register);

        // Get table key based on the index type (string or number)
        viaTableKey key = viaT_checkstring(V, idx) ? idx.val_string->hash : static_cast<viaTableKey>(idx.val_number);

        // Assert that the value is a table
        std::string _errfmt = std::format("Attempt to load index of {}", via_type(V, tbl).val_string->ptr);
        VM_ASSERT(viaT_checktable(V, tbl), _errfmt);

        // Load the table index
        via_loadtableindex(V, tbl.val_table, key, rdst.val_register);
        VM_NEXT();
    }

    case OpCode::SETIDX:
    {
        viaOperand rsrc = VM_OPND(0);
        viaOperand rtbl = VM_OPND(1);
        viaOperand ridx = VM_OPND(2);

        viaValue val;
        viaValue tbl = *via_getregister(V, rtbl.val_register);
        viaValue idx = *via_getregister(V, ridx.val_register);

        // Get table key based on the index type (string or number)
        viaTableKey key = viaT_checkstring(V, idx) ? idx.val_string->hash : static_cast<viaHash_t>(idx.val_number);

        // Assert that the value is a table
        std::string _errfmt = std::format("Attempt to assign index to {}", ENUM_NAME(tbl.type));
        VM_ASSERT(viaT_checktable(V, tbl), _errfmt);

        // Slow-path: the value is stored in a register, load it
        if (VIA_UNLIKELY(rsrc.type == viaOperandType_t::Register))
            val = *via_getregister(V, rsrc.val_register);
        else
            val = via_toviavalue(V, rsrc);

        // Set the table index
        viaTable *T = tbl.val_table;
        via_settableindex(V, T, key, val);

        VM_NEXT();
    }

    case OpCode::LEN:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand objr = VM_OPND(1);

        viaValue val;

        // Fast-path: object is stored in a register
        if (VIA_LIKELY(objr.type == viaOperandType_t::Register))
            val = *via_getregister(V, objr.val_register);
        else
            val = via_toviavalue(V, objr);

        via_setregister(V, rdst.val_register, via_len(V, val));

        VM_NEXT();
    }

    case OpCode::TOSTRING:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand valr = VM_OPND(1);

        viaValue val = *via_getregister(V, valr.val_register);

        via_setregister(V, rdst.val_register, val);

        VM_NEXT();
    }

    case OpCode::TONUMBER:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand valr = VM_OPND(1);

        viaValue val = *via_getregister(V, valr.val_register);
        via_setregister(V, rdst.val_register, via_tonumber(V, val));

        VM_NEXT();
    }

    case OpCode::TOBOOL:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand valr = VM_OPND(1);

        viaValue val = *via_getregister(V, valr.val_register);
        via_setregister(V, rdst.val_register, via_tobool(V, val));

        VM_NEXT();
    }

    case OpCode::TYPE:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand objr = VM_OPND(1);

        viaValue *val = via_getregister(V, objr.val_register);
        viaValue type = via_type(V, *val);

        via_setregister(V, rdst.val_register, type);

        VM_NEXT();
    }

    case OpCode::TYPEOF:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand objr = VM_OPND(1);

        viaValue *val = via_getregister(V, objr.val_register);
        viaValue type = via_typeof(V, *val);

        via_setregister(V, rdst.val_register, type);

        VM_NEXT();
    }

    case OpCode::ISNIL:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand objr = VM_OPND(1);

        bool is_nil = via_getregister(V, objr.val_register)->type == viaValueType::Nil;
        via_setregister(V, rdst.val_register, viaT_stackvalue(V, is_nil));

        VM_NEXT();
    }

    case OpCode::STRCON:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand lhsr = VM_OPND(1);
        viaOperand rhsr = VM_OPND(2);

        viaValue lhs = *via_getregister(V, lhsr.val_register);
        viaValue rhs = *via_getregister(V, rhsr.val_register);

        VM_ASSERT(viaT_checkstring(V, lhs), "Attempt to concatenate non-string value");
        VM_ASSERT(viaT_checkstring(V, rhs), "Attempt to concatenate string with non-string value");

        std::string str = std::string(lhs.val_string->ptr) + rhs.val_string->ptr;
        viaString *vstr = viaT_newstring(V, str.c_str());

        via_setregister(V, rdst.val_register, viaT_stackvalue(V, vstr));

        VM_NEXT();
    }

    default:
        via_setexitdata(V, 1, std::format("Unrecognized OpCode (op_id={})", static_cast<uint8_t>(V->ip->op)));
        VM_EXIT();
    }
}

exit:
    std::cout << std::format("VM exiting with exit_code={}, exit_message={}\n", V->exitc, V->exitm);
}

void via_killthread(viaState *V)
{
    if (V->tstate == viaThreadState::RUNNING)
        // TODO: Wait for the VM to exit
        V->abrt = true;

    // Mark as dead thread
    V->tstate = viaThreadState::DEAD;
    // Decrement the thread_id to make room for more threads (I know you can technically make 4 billion threads ok?)
    __thread_id__--;
}

void via_pausethread(viaState *V)
{
    V->tstate = viaThreadState::PAUSED;
    // Save the VM state to restore it when paused
    via_savestate(V);
}

} // namespace via