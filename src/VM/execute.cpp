/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

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
#include "debug.h"
#include <chrono>
#include <cmath>
#include <cstdint>

// Define the hot path threshold for the instruction dispatch loop
// How many times an instruction will be executed before being flagged as "hot"
#ifndef VIA_HOTPATH_THRESHOLD
#    define VIA_HOTPATH_THRESHOLD 64
#endif

// Check for debug mode and warn the user about it's possible effects on behavior
#ifdef VIA_DEBUG
#    warning Compiling via in debug mode; optimizations will be disabled!
#    ifdef VIA_ALLOW_OPTIMIZATIONS_IN_DEBUG_MODE
#        warning Enabling optimizations in debug mode may cause instability issues!
#    endif
#endif
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

// Get compilation stuff in the namespace
using namespace Compilation;

// Check if the instruction holds an empty OpCode, e.g. NOP or END
// Used for runtime optimizations
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
            V->ip->operandv[0] = {.type = viaOperandType_t::Number, .val_number = static_cast<double>(skip_count)};
        }
    }
}

// This does not need to be inline
// Becaue it should only be called once
void via_execute(viaState *V)
{
    VIA_ASSERT(V->tstate != viaThreadState::RUNNING, "via_execute() called on running thread (tstate=RUNNING)");
    VIA_ASSERT(V->tstate != viaThreadState::DEAD, "via_execute() called on dead thread (tstate=DEAD)");

    V->tstate = viaThreadState::RUNNING;

    // Not necessary, but provides clarity
    VM_DISPATCH();

dispatch:
{
    // Increment path counter of this specific instruction
    V->ip->pc++;

    // Flag the instruction as a "hot" instruction if it has been executed more than a certain amount
    if (VIA_UNLIKELY(!V->ip->hot && V->ip->pc >= VIA_HOTPATH_THRESHOLD))
        V->ip->hot = true;

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

#ifdef VIA_DEBUG
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
#endif

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
#ifdef VIA_DEBUG
    case OpCode::ERR:
    {
        via_setexitdata(V, 1, "ERR OpCode");
        VM_EXIT();
    }
#endif

    case OpCode::END:
    case OpCode::NOP:
    {
        // Optimizations that modify the program are not allowed in debug mode
        // Unless a very certain flag is provided
#if !defined(VIA_DEBUG) || defined(VIA_DEBUG) && defined(VIA_ALLOW_OPTIMIZATIONS_IN_DEBUG_MODE)
        // Attempt to optimize empty instruction sequence
        _optimize_empty_instruction_sequence(V);
#endif
        VM_NEXT();
    }

    case OpCode::MOV:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand rsrc = VM_OPND(1);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rsrc), "Expected Register for MOV source");
#endif
        // Fast-path: both operands are registers
        if (VIA_LIKELY(viaC_checkregister(rdst)))
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
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rdst), "Expected Register for CPY destination");
        VM_ASSERT(viaC_checkregister(rsrc), "Expected Register for CPY source");
#endif
        // This has to be copied, otherwise it will remain a reference
        viaValue cpy = *via_getregister(V, rsrc.val_register);
        via_setregister(V, rdst.val_register, cpy);

        VM_NEXT();
    }

    case OpCode::LI:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand imm = VM_OPND(1);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rdst), "Expected Register for LI destination");
#endif
        viaValue val = via_toviavalue(V, imm);
        via_setregister(V, rdst.val_register, val);
        VM_NEXT();
    }

    case OpCode::NIL:
    {
        viaOperand rdst = VM_OPND(0);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rdst), "Expected Register for NIL destination");
#endif
        via_setregister(V, rdst.val_register, viaT_stackvalue(V));
        VM_NEXT();
    }

    case OpCode::PUSH:
    {
        viaFunction *frame = new viaFunction{
            0,
            false,
            false,
            "LC",
            viaS_top(V->stack),
            {},
            {},
        };

        viaS_push(V->stack, frame);
        VM_NEXT();
    }

    case OpCode::POP:
    {
#ifdef VIA_DEBUG
        if (V->stack->size == 1)
        {
            via_setexitdata(V, 1, "Attempt to POP global (root) stack frame");
            VM_EXIT();
        }
#endif
        viaS_pop(V->stack);
        VM_NEXT();
    }

    case OpCode::PUSHARG:
    {
        viaOperand arg = VM_OPND(0);
        viaValue arg_val;

        if (VIA_LIKELY(viaC_checkregister(arg)))
            arg_val = *via_getregister(V, arg.val_register);
        else
            arg_val = via_toviavalue(V, arg);

        viaS_push(V->arguments, arg_val);
        VM_NEXT();
    }

    case OpCode::POPARG:
    {
        viaOperand dst = VM_OPND(0);
        viaValue val = viaS_top(V->arguments);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(dst), "Expected register for POPARG destination");
#endif
        viaS_pop(V->arguments);
        via_setregister(V, dst.val_register, val);
        VM_NEXT();
    }

    case OpCode::PUSHRET:
    {
        viaOperand ret = VM_OPND(0);
        viaValue ret_val;

        if (VIA_LIKELY(viaC_checkregister(ret)))
            ret_val = *via_getregister(V, ret.val_register);
        else
            ret_val = via_toviavalue(V, ret);

        viaS_push(V->returns, ret_val);
        VM_NEXT();
    }

    case OpCode::POPRET:
    {
        viaOperand dst = VM_OPND(0);
        viaValue val = viaS_top(V->returns);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(dst), "Expected register for POPRET destination");
#endif
        viaS_pop(V->returns);
        via_setregister(V, dst.val_register, val);
        VM_NEXT();
    }

    case OpCode::SETLOCAL:
    {
        viaOperand id = VM_OPND(0);
        viaOperand val = VM_OPND(1);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkidentifier(id), "Expected identifier for SETLOCAL id");
#endif
        viaVariableIdentifier_t id_t = viaT_hashstring(V, id.val_identifier);
        // Slow-path: loaded value is a register
        if (VIA_UNLIKELY(viaC_checkregister(val)))
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
        if (VIA_UNLIKELY(viaC_checkregister(val)))
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

        viaValue global = via_getvariable(V, viaT_hashstring(V, id.val_identifier));
        via_setregister(V, dst.val_register, global);

        VM_NEXT();
    }

    case OpCode::ADD:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand rlhs = VM_OPND(1);
        viaOperand rrhs = VM_OPND(2);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rdst), "Expected register for ADD destination");
        VM_ASSERT(viaC_checkregister(rlhs), "Expected register for ADD lhs");
        VM_ASSERT(viaC_checkregister(rrhs), "Expected register for ADD rhs");
#endif
        viaValue *lhs = via_getregister(V, rlhs.val_register);
        viaValue *rhs = via_getregister(V, rrhs.val_register);

        if (VIA_LIKELY(viaT_checknumber(V, *lhs)))
        {
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checknumber(V, *rhs), "Expected Number for ADD rvalue");
#endif
            via_setregister(V, rdst.val_register, viaT_stackvalue(V, lhs->val_number + rhs->val_number));
        }
        else if (VIA_UNLIKELY(viaT_checktable(V, *lhs)))
        {
            viaValue *mm = via_getmetamethod(V, *lhs, OpCode::ADD);
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checkcallable(V, *mm), "Expected Callable metamethod for ADD lvalue");
#endif
            via_pushargument(V, *rhs);
            via_call(V, *mm);
            via_setregister(V, rdst.val_register, via_popreturn(V));
        }
#ifdef VIA_DEBUG
        else
            VM_ASSERT(false, "Expected valid lvalue for ADD");
#endif
        VM_NEXT();
    }
    case OpCode::SUB:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand rlhs = VM_OPND(1);
        viaOperand rrhs = VM_OPND(2);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rdst), "Expected register for SUB destination");
        VM_ASSERT(viaC_checkregister(rlhs), "Expected register for SUB lhs");
        VM_ASSERT(viaC_checkregister(rrhs), "Expected register for SUB rhs");
#endif
        viaValue *lhs = via_getregister(V, rlhs.val_register);
        viaValue *rhs = via_getregister(V, rrhs.val_register);

        if (VIA_LIKELY(viaT_checknumber(V, *lhs)))
        {
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checknumber(V, *rhs), "Expected Number for SUB rvalue");
#endif
            via_setregister(V, rdst.val_register, viaT_stackvalue(V, lhs->val_number - rhs->val_number));
        }
        else if (VIA_UNLIKELY(viaT_checktable(V, *lhs)))
        {
            viaValue *mm = via_getmetamethod(V, *lhs, OpCode::SUB);
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checkcallable(V, *mm), "Expected Callable metamethod for SUB lvalue");
#endif
            via_pushargument(V, *rhs);
            via_call(V, *mm);
            via_setregister(V, rdst.val_register, via_popreturn(V));
        }
#ifdef VIA_DEBUG
        else
            VM_ASSERT(false, "Expected valid lvalue for SUB");
#endif
        VM_NEXT();
    }
    case OpCode::MUL:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand rlhs = VM_OPND(1);
        viaOperand rrhs = VM_OPND(2);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rdst), "Expected register for MUL destination");
        VM_ASSERT(viaC_checkregister(rlhs), "Expected register for MUL lhs");
        VM_ASSERT(viaC_checkregister(rrhs), "Expected register for MUL rhs");
#endif
        viaValue *lhs = via_getregister(V, rlhs.val_register);
        viaValue *rhs = via_getregister(V, rrhs.val_register);

        if (VIA_LIKELY(viaT_checknumber(V, *lhs)))
        {
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checknumber(V, *rhs), "Expected Number for MUL rvalue");
#endif
            via_setregister(V, rdst.val_register, viaT_stackvalue(V, lhs->val_number * rhs->val_number));
        }
        else if (VIA_UNLIKELY(viaT_checktable(V, *lhs)))
        {
            viaValue *mm = via_getmetamethod(V, *lhs, OpCode::MUL);
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checkcallable(V, *mm), "Expected Callable metamethod for MUL lvalue");
#endif
            via_pushargument(V, *rhs);
            via_call(V, *mm);
            via_setregister(V, rdst.val_register, via_popreturn(V));
        }
#ifdef VIA_DEBUG
        else
            VM_ASSERT(false, "Expected valid lvalue for MUL");
#endif
        VM_NEXT();
    }
    case OpCode::DIV:
    {
        viaOperand rdst = VM_OPND(0);
        viaOperand rlhs = VM_OPND(1);
        viaOperand rrhs = VM_OPND(2);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rdst), "Expected register for DIV destination");
        VM_ASSERT(viaC_checkregister(rlhs), "Expected register for DIV lhs");
        VM_ASSERT(viaC_checkregister(rrhs), "Expected register for DIV rhs");
#endif
        viaValue *lhs = via_getregister(V, rlhs.val_register);
        viaValue *rhs = via_getregister(V, rrhs.val_register);

        if (VIA_LIKELY(viaT_checknumber(V, *lhs)))
        {
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checknumber(V, *rhs), "Expected Number for DIV rvalue");
#endif
            via_setregister(V, rdst.val_register, viaT_stackvalue(V, lhs->val_number / rhs->val_number));
        }
        else if (VIA_UNLIKELY(viaT_checktable(V, *lhs)))
        {
            viaValue *mm = via_getmetamethod(V, *lhs, OpCode::DIV);
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checkcallable(V, *mm), "Expected Callable metamethod for DIV lvalue");
#endif
            via_pushargument(V, *rhs);
            via_call(V, *mm);
            via_setregister(V, rdst.val_register, via_popreturn(V));
        }
#ifdef VIA_DEBUG
        else
            VM_ASSERT(false, "Expected valid lvalue for DIV");
#endif
        VM_NEXT();
    }

    case OpCode::IADD:
    {
        viaOperand rlhs = VM_OPND(0);
        viaOperand rrhs = VM_OPND(1);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rlhs), "Expected register for IADD lhs");
        VM_ASSERT(viaC_checkregister(rrhs), "Expected register for IADD rhs");
#endif
        viaValue *lhs = via_getregister(V, rlhs.val_register);
        viaValue *rhs = via_getregister(V, rrhs.val_register);

        if (VIA_LIKELY(viaT_checknumber(V, *lhs)))
        {
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checknumber(V, *rhs), "Expected Number for IADD rvalue");
#endif
            lhs->val_number += rhs->val_number;
        }
        else if (VIA_UNLIKELY(viaT_checktable(V, *lhs)))
        {
            viaValue *mm = via_getmetamethod(V, *lhs, OpCode::ADD);
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checkcallable(V, *mm), "Expected Callable metamethod for IADD lvalue");
#endif
            via_pushargument(V, *rhs);
            via_call(V, *mm);
            *lhs = via_popreturn(V);
        }
#ifdef VIA_DEBUG
        else
            VM_ASSERT(false, "Expected valid lvalue for IADD");
#endif
        VM_NEXT();
    }
    case OpCode::ISUB:
    {
        viaOperand rlhs = VM_OPND(0);
        viaOperand rrhs = VM_OPND(1);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rlhs), "Expected register for ISUB lhs");
        VM_ASSERT(viaC_checkregister(rrhs), "Expected register for ISUB rhs");
#endif
        viaValue *lhs = via_getregister(V, rlhs.val_register);
        viaValue *rhs = via_getregister(V, rrhs.val_register);

        if (VIA_LIKELY(viaT_checknumber(V, *lhs)))
        {
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checknumber(V, *rhs), "Expected Number for ISUB rvalue");
#endif
            lhs->val_number -= rhs->val_number;
        }
        else if (VIA_UNLIKELY(viaT_checktable(V, *lhs)))
        {
            viaValue *mm = via_getmetamethod(V, *lhs, OpCode::SUB);
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checkcallable(V, *mm), "Expected Callable metamethod for ISUB lvalue");
#endif
            via_pushargument(V, *rhs);
            via_call(V, *mm);
            *lhs = via_popreturn(V);
        }
#ifdef VIA_DEBUG
        else
            VM_ASSERT(false, "Expected valid lvalue for ISUB");
#endif
        VM_NEXT();
    }
    case OpCode::IMUL:
    {
        viaOperand rlhs = VM_OPND(0);
        viaOperand rrhs = VM_OPND(1);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rlhs), "Expected register for IMUL lhs");
        VM_ASSERT(viaC_checkregister(rrhs), "Expected register for IMUL rhs");
#endif
        viaValue *lhs = via_getregister(V, rlhs.val_register);
        viaValue *rhs = via_getregister(V, rrhs.val_register);

        if (VIA_LIKELY(viaT_checknumber(V, *lhs)))
        {
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checknumber(V, *rhs), "Expected Number for IMUL rvalue");
#endif
            lhs->val_number *= rhs->val_number;
        }
        else if (VIA_UNLIKELY(viaT_checktable(V, *lhs)))
        {
            viaValue *mm = via_getmetamethod(V, *lhs, OpCode::MUL);
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checkcallable(V, *mm), "Expected Callable metamethod for IMUL lvalue");
#endif
            via_pushargument(V, *rhs);
            via_call(V, *mm);
            *lhs = via_popreturn(V);
        }
#ifdef VIA_DEBUG
        else
            VM_ASSERT(false, "Expected valid lvalue for IADD");
#endif
        VM_NEXT();
    }
    case OpCode::IDIV:
    {
        viaOperand rlhs = VM_OPND(0);
        viaOperand rrhs = VM_OPND(1);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rlhs), "Expected register for IDIV lhs");
        VM_ASSERT(viaC_checkregister(rrhs), "Expected register for IDIV rhs");
#endif
        viaValue *lhs = via_getregister(V, rlhs.val_register);
        viaValue *rhs = via_getregister(V, rrhs.val_register);

        if (VIA_LIKELY(viaT_checknumber(V, *lhs)))
        {
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checknumber(V, *rhs), "Expected Number for IDIV rvalue");
#endif
            lhs->val_number /= rhs->val_number;
        }
        else if (VIA_UNLIKELY(viaT_checktable(V, *lhs)))
        {
            viaValue *mm = via_getmetamethod(V, *lhs, OpCode::DIV);
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checkcallable(V, *mm), "Expected Callable metamethod for IDIV lvalue");
#endif
            via_pushargument(V, *rhs);
            via_call(V, *mm);
            *lhs = via_popreturn(V);
        }
#ifdef VIA_DEBUG
        else
            VM_ASSERT(false, "Expected valid lvalue for IDIV");
#endif
        VM_NEXT();
    }

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
#ifdef VIA_DEBUG
        VM_ASSERT(viaT_checknumber(V, lhsn), "Expected Number for binary operand 0");
#endif
        via_setregister(V, dst.val_register, viaT_stackvalue(V, -lhsn.val_number));

        VM_NEXT();
    }

    case OpCode::BAND:
    {
        viaOperand dst = VM_OPND(0);
        viaOperand lhs = VM_OPND(1);
        viaOperand rhs = VM_OPND(2);

        viaValue lhsn, rhsn;

        if (VIA_LIKELY(lhs.type == viaOperandType_t::Bool))
            lhsn.val_boolean = lhs.val_boolean;
        else
            lhsn = *via_getregister(V, lhs.val_register);

        if (VIA_LIKELY(rhs.type == viaOperandType_t::Bool))
            rhsn.val_boolean = rhs.val_boolean;
        else
            rhsn = *via_getregister(V, rhs.val_register);

        bool result = via_tobool(V, lhsn).val_boolean && via_tobool(V, rhsn).val_boolean;
        via_setregister(V, dst.val_register, viaT_stackvalue(V, result));
        VM_NEXT();
    }
    case OpCode::BOR:
    {
        viaOperand dst = VM_OPND(0);
        viaOperand lhs = VM_OPND(1);
        viaOperand rhs = VM_OPND(2);

        viaValue lhsn, rhsn;

        if (VIA_LIKELY(lhs.type == viaOperandType_t::Bool))
            lhsn.val_boolean = lhs.val_boolean;
        else
            lhsn = *via_getregister(V, lhs.val_register);

        if (VIA_LIKELY(rhs.type == viaOperandType_t::Bool))
            rhsn.val_boolean = rhs.val_boolean;
        else
            rhsn = *via_getregister(V, rhs.val_register);

        bool result = via_tobool(V, lhsn).val_boolean || via_tobool(V, rhsn).val_boolean;
        via_setregister(V, dst.val_register, viaT_stackvalue(V, result));
        VM_NEXT();
    }
    case OpCode::BXOR:
    {
        viaOperand dst = VM_OPND(0);
        viaOperand lhs = VM_OPND(1);
        viaOperand rhs = VM_OPND(2);

        viaValue lhsn, rhsn;

        if (VIA_LIKELY(lhs.type == viaOperandType_t::Bool))
            lhsn.val_boolean = lhs.val_boolean;
        else
            lhsn = *via_getregister(V, lhs.val_register);

        if (VIA_LIKELY(rhs.type == viaOperandType_t::Bool))
            rhsn.val_boolean = rhs.val_boolean;
        else
            rhsn = *via_getregister(V, rhs.val_register);

        bool result = via_tobool(V, lhsn).val_boolean != via_tobool(V, rhsn).val_boolean;
        via_setregister(V, dst.val_register, viaT_stackvalue(V, result));
        VM_NEXT();
    }

    case OpCode::BNOT:
    {
        viaOperand dst = VM_OPND(0);
        viaOperand lhs = VM_OPND(1);
        viaValue lhsn = *via_getregister(V, lhs.val_register);
#ifdef VIA_DEBUG
        VM_ASSERT(viaT_checkbool(V, lhsn), "Expected Bool for logical operand 0");
#endif
        via_setregister(V, dst.val_register, viaT_stackvalue(V, !lhsn.val_boolean));

        VM_NEXT();
    }

    case OpCode::EQ:
    {
        viaOperand dst = VM_OPND(0);
        viaOperand lhs = VM_OPND(1);
        viaOperand rhs = VM_OPND(2);

        viaValue lhsn, rhsn;

        bool lhs_reg = false;
        bool rhs_reg = false;

        if (VIA_UNLIKELY(lhs.type == viaOperandType_t::Register))
        {
            lhsn = *via_getregister(V, lhs.val_register);
            lhs_reg = true;
        }
        else
            lhsn = via_toviavalue(V, lhs);

        if (VIA_UNLIKELY(rhs.type == viaOperandType_t::Register))
        {
            rhsn = *via_getregister(V, rhs.val_register);
            rhs_reg = true;
        }
        else
            rhsn = via_toviavalue(V, rhs);

        if (lhs_reg && rhs_reg)
            via_setregister(V, dst.val_register, viaT_stackvalue(V, via_cmpregister(V, lhs.val_register, rhs.val_register)));
        else if (lhs_reg)
            via_setregister(V, dst.val_register, viaT_stackvalue(V, via_compare(V, *via_getregister(V, lhs.val_register), rhsn)));
        else if (rhs_reg)
            via_setregister(V, dst.val_register, viaT_stackvalue(V, via_compare(V, *via_getregister(V, rhs.val_register), lhsn)));
        else
            via_setregister(V, dst.val_register, viaT_stackvalue(V, via_compare(V, lhsn, rhsn)));

        VM_NEXT();
    }
    case OpCode::NEQ:
    {
        viaOperand dst = VM_OPND(0);
        viaOperand lhs = VM_OPND(1);
        viaOperand rhs = VM_OPND(2);

        viaValue lhsn, rhsn;

        bool lhs_reg = false;
        bool rhs_reg = false;

        if (VIA_UNLIKELY(lhs.type == viaOperandType_t::Register))
        {
            lhsn = *via_getregister(V, lhs.val_register);
            lhs_reg = true;
        }
        else
            lhsn = via_toviavalue(V, lhs);

        if (VIA_UNLIKELY(rhs.type == viaOperandType_t::Register))
        {
            rhsn = *via_getregister(V, rhs.val_register);
            rhs_reg = true;
        }
        else
            rhsn = via_toviavalue(V, rhs);

        if (lhs_reg && rhs_reg)
            via_setregister(V, dst.val_register, viaT_stackvalue(V, !via_cmpregister(V, lhs.val_register, rhs.val_register)));
        else if (lhs_reg)
            via_setregister(V, dst.val_register, viaT_stackvalue(V, !via_compare(V, *via_getregister(V, lhs.val_register), rhsn)));
        else if (rhs_reg)
            via_setregister(V, dst.val_register, viaT_stackvalue(V, !via_compare(V, *via_getregister(V, rhs.val_register), lhsn)));
        else
            via_setregister(V, dst.val_register, viaT_stackvalue(V, !via_compare(V, lhsn, rhsn)));

        VM_NEXT();
    }
    case OpCode::LT:
    {
        viaOperand dst = VM_OPND(0);
        viaOperand lhs = VM_OPND(1);
        viaOperand rhs = VM_OPND(2);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(dst), "Expected register for LT destination");
        VM_ASSERT(viaC_checkregister(lhs), "Expected register for LT lhs");
        VM_ASSERT(viaC_checkregister(rhs), "Expected register for LT rhs");
#endif
        viaValue lhsn = *via_getregister(V, lhs.val_register);
        viaValue rhsn = *via_getregister(V, rhs.val_register);
#ifdef VIA_DEBUG
        VM_ASSERT(viaT_checknumber(V, rhsn), "Expected Number for LT rvalue");
#endif
        if (VIA_LIKELY(viaT_checknumber(V, lhsn)))
        {
            via_setregister(V, dst.val_register, viaT_stackvalue(V, lhsn.val_number < rhsn.val_number));
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(viaT_checktable(V, lhsn)))
        {
            viaValue *mm = via_getmetamethod(V, lhsn, OpCode::LT);
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checkcallable(V, *mm), "Expected callable metamethod for LT lvalue");
#endif
            via_pushargument(V, rhsn);
            via_call(V, *mm);
            via_setregister(V, dst.val_register, via_popargument(V));
            VM_NEXT();
        }
#ifdef VIA_DEBUG
        else
            VM_ASSERT(false, "Expected valid lvalue for LT");
#endif
        VM_NEXT();
    }
    case OpCode::GT:
    {
        viaOperand dst = VM_OPND(0);
        viaOperand lhs = VM_OPND(1);
        viaOperand rhs = VM_OPND(2);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(dst), "Expected register for GT destination");
        VM_ASSERT(viaC_checkregister(lhs), "Expected register for GT lhs");
        VM_ASSERT(viaC_checkregister(rhs), "Expected register for GT rhs");
#endif
        viaValue lhsn = *via_getregister(V, lhs.val_register);
        viaValue rhsn = *via_getregister(V, rhs.val_register);
#ifdef VIA_DEBUG
        VM_ASSERT(viaT_checknumber(V, rhsn), "Expected Number for GT rvalue");
#endif
        if (VIA_LIKELY(viaT_checknumber(V, lhsn)))
        {
            via_setregister(V, dst.val_register, viaT_stackvalue(V, lhsn.val_number > rhsn.val_number));
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(viaT_checktable(V, lhsn)))
        {
            viaValue *mm = via_getmetamethod(V, lhsn, OpCode::GT);
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checkcallable(V, *mm), "Expected callable metamethod for GT lvalue");
#endif
            via_pushargument(V, rhsn);
            via_call(V, *mm);
            via_setregister(V, dst.val_register, via_popargument(V));
            VM_NEXT();
        }
#ifdef VIA_DEBUG
        else
            VM_ASSERT(false, "Expected valid lvalue for GT");
#endif
        VM_NEXT();
    }
    case OpCode::LE:
    {
        viaOperand dst = VM_OPND(0);
        viaOperand lhs = VM_OPND(1);
        viaOperand rhs = VM_OPND(2);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(dst), "Expected register for LE destination");
        VM_ASSERT(viaC_checkregister(lhs), "Expected register for LE lhs");
        VM_ASSERT(viaC_checkregister(rhs), "Expected register for LE rhs");
#endif
        viaValue lhsn = *via_getregister(V, lhs.val_register);
        viaValue rhsn = *via_getregister(V, rhs.val_register);
#ifdef VIA_DEBUG
        VM_ASSERT(viaT_checknumber(V, rhsn), "Expected Number for LE rvalue");
#endif
        if (VIA_LIKELY(viaT_checknumber(V, lhsn)))
        {
            via_setregister(V, dst.val_register, viaT_stackvalue(V, lhsn.val_number <= rhsn.val_number));
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(viaT_checktable(V, lhsn)))
        {
            viaValue *mm = via_getmetamethod(V, lhsn, OpCode::LE);
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checkcallable(V, *mm), "Expected callable metamethod for LE lvalue");
#endif
            via_pushargument(V, rhsn);
            via_call(V, *mm);
            via_setregister(V, dst.val_register, via_popargument(V));
            VM_NEXT();
        }
#ifdef VIA_DEBUG
        else
            VM_ASSERT(false, "Expected valid lvalue for LE");
#endif
        VM_NEXT();
    }
    case OpCode::GE:
    {
        viaOperand dst = VM_OPND(0);
        viaOperand lhs = VM_OPND(1);
        viaOperand rhs = VM_OPND(2);
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(dst), "Expected register for GE destination");
        VM_ASSERT(viaC_checkregister(lhs), "Expected register for GE lhs");
        VM_ASSERT(viaC_checkregister(rhs), "Expected register for GE rhs");
#endif
        viaValue lhsn = *via_getregister(V, lhs.val_register);
        viaValue rhsn = *via_getregister(V, rhs.val_register);
#ifdef VIA_DEBUG
        VM_ASSERT(viaT_checknumber(V, rhsn), "Expected Number for GE rvalue");
#endif
        if (VIA_LIKELY(viaT_checknumber(V, lhsn)))
        {
            via_setregister(V, dst.val_register, viaT_stackvalue(V, lhsn.val_number >= rhsn.val_number));
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(viaT_checktable(V, lhsn)))
        {
            viaValue *mm = via_getmetamethod(V, lhsn, OpCode::GE);
#ifdef VIA_DEBUG
            VM_ASSERT(viaT_checkcallable(V, *mm), "Expected callable metamethod for GE lvalue");
#endif
            via_pushargument(V, rhsn);
            via_call(V, *mm);
            via_setregister(V, dst.val_register, via_popargument(V));
            VM_NEXT();
        }
#ifdef VIA_DEBUG
        else
            VM_ASSERT(false, "Expected valid lvalue for GE");
#endif
        VM_NEXT();
    }

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
#ifdef VIA_DEBUG
        VM_ASSERT(viaT_checkptr(V, addrv), "Expected Ptr for GCADD");
#endif
        via_gcadd(V, reinterpret_cast<viaValue *>(addrv.val_pointer));

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
#ifdef VIA_DEBUG
        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));
#endif
        VM_JMPTO(it->second);
        VM_NEXT();
    }

    case OpCode::JLZ:
    case OpCode::JLNZ:
    {
        viaOperand valr = VM_OPND(0);
        viaOperand label = VM_OPND(1);

        auto it = V->labels->find(std::string_view(label.val_identifier));
#ifdef VIA_DEBUG
        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));
#endif
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
#ifdef VIA_DEBUG
        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));
#endif
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
#ifdef VIA_DEBUG
        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));
#endif
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
#ifdef VIA_DEBUG
        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));
#endif
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
#ifdef VIA_DEBUG
        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));
#endif
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
#ifdef VIA_DEBUG
        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));
#endif
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
        viaLabelKey_t key = id.val_identifier;
        viaInstruction *instr = V->ip;

        auto it = V->labels->find(key);
        if (it != V->labels->end())
            (*V->labels)[key] = instr;

        while (V->ip < V->ibp)
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
#ifdef VIA_DEBUG
        // Assert that the value is a table
        std::string _errfmt = std::format("Attempt to index {} with '{}'", via_type(V, tbl).val_string->ptr, key);
        VM_ASSERT(viaT_checktable(V, tbl), _errfmt);
#endif
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
#ifdef VIA_DEBUG
        // Assert that the value is a table
        std::string _errfmt = std::format("Attempt to load index of {}", via_type(V, tbl).val_string->ptr);
        VM_ASSERT(viaT_checktable(V, tbl), _errfmt);
#endif
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
#ifdef VIA_DEBUG
        // Assert that the value is a table
        std::string _errfmt = std::format("Attempt to assign index to {}", ENUM_NAME(tbl.type));
        VM_ASSERT(viaT_checktable(V, tbl), _errfmt);
#endif
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
#ifdef VIA_DEBUG
        VM_ASSERT(viaT_checkstring(V, lhs), "Attempt to concatenate non-string value");
        VM_ASSERT(viaT_checkstring(V, rhs), "Attempt to concatenate string with non-string value");
#endif
        std::string str = std::string(lhs.val_string->ptr) + rhs.val_string->ptr;
        viaString *vstr = viaT_newstring(V, str.c_str());

        via_setregister(V, rdst.val_register, viaT_stackvalue(V, vstr));

        VM_NEXT();
    }

    case OpCode::DEBUGREGISTERS:
    {
        viaOperand count = VM_OPND(0);
#ifdef VIA_DEBUG
        VM_ASSERT(count.type == viaOperandType_t::Number, "Expected number of registers to debug");
#endif
        viaD_printregistermap(V, static_cast<size_t>(count.val_number));
        VM_NEXT();
    }

    case OpCode::DEBUGARGUMENTS:
    {
        viaOperand count = VM_OPND(0);
#ifdef VIA_DEBUG
        VM_ASSERT(count.type == viaOperandType_t::Number, "Expected number of arguments to debug");
#endif
        viaD_printargumentstack(V, static_cast<size_t>(count.val_number));
        VM_NEXT();
    }

    case OpCode::DEBUGRETURNS:
    {
        viaOperand count = VM_OPND(0);
#ifdef VIA_DEBUG
        VM_ASSERT(count.type == viaOperandType_t::Number, "Expected number of returns to debug");
#endif
        viaD_printreturnstack(V, static_cast<size_t>(count.val_number));
        VM_NEXT();
    }

    default:
        via_setexitdata(V, 1, std::format("Unrecognized OpCode (op_id={})", static_cast<uint8_t>(V->ip->op)));
        VM_EXIT();
    }
}

exit:
    std::cout << std::format("VM exiting with exit_code={}, exit_message={}\n", V->exitc, V->exitm);
} // namespace via

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