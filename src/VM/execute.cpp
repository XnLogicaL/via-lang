/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

/*
 * !! WARNING !!
 * Unit testing is not allowed in this file!
 * This is due to the performance critical nature of it's contents
 */

#include "execute.h"
#include "api.h"
#include "chunk.h"
#include "execlinux.h"
#include "execwin.h"
#include "core.h"
#include "shared.h"
#include "state.h"
#include "types.h"
#include "debug.h"
#include <chrono>
#include <cstdint>

// Define the hot path threshold for the instruction dispatch loop
// How many times an instruction will be executed before being flagged as "hot"
#ifndef VIA_HOTPATH_THRESHOLD
#    define VIA_HOTPATH_THRESHOLD 64
#endif

// Check for debug mode and warn the user about it's possible effects on behavior
#ifdef VIA_DEBUG
#    pragma message(Compiling via in debug mode; some optimizations may be disabled)
#    ifdef VIA_ALLOW_OPTIMIZATIONS_IN_DEBUG_MODE
#        pragma message(Enabling optimizations in debug mode may cause instability issues)
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

namespace via
{

// Get compilation stuff in the namespace
using namespace Compilation;

// Check if the instruction holds an empty OpCode, e.g. NOP or END
// Used for runtime optimizations
inline bool _is_empty_instruction(Instruction *instr)
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
        JmpOffset skip_count = 1;

        // Find the end of the sequence of empty instructions
        while (V->ip + skip_count < V->ibp && _is_empty_instruction(V->ip + skip_count))
            skip_count++;

        // If there are multiple empty instructions, replace the first one with a JMP
        if (skip_count > 1)
        {
            V->ip->op = OpCode::JMP;
            V->ip->operand1 = {.type = OperandType::Number, .val_number = static_cast<double>(skip_count)};
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
    // Check if the instruction pointer is the start of a chunk
    if (VIA_UNLIKELY(V->ip->chunk != nullptr))
    {
        // Increment chunk program counter
        V->ip->chunk->pc++;
        // Check if the program counter exceeds a certain threshold
        // to determine if it needs to be compiled into machine code
        if (V->ip->chunk->pc >= VIA_HOTPATH_THRESHOLD)
        {
            // This function automatically compiles the chunk if it hasn't been compiled before
            // Disabled for now due to machine code instability
            // jit::viaJIT_executechunk(V, V->ip->chunk);
        }

        // After the chunk has been executed,
        // we need to find the next chunk and jump to it
        for (Instruction *i = V->ip; i <= V->ihp; i++)
            if (i->chunk != nullptr)
                break;
    }

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
        // Otherwise, this is completely impossible to be produced by the compiler
        VIA_UNLIKELY(via_validjmpaddr(V, V->ip)),
        std::format(
            "Instruction pointer out of bounds (ip={}, ihp={}, ibp={})",
            reinterpret_cast<const void *>(V->ip),
            reinterpret_cast<const void *>(V->ihp),
            reinterpret_cast<const void *>(V->ibp)
        )
    );
#endif

    // This is unlikely because the VM very rarely yields at all
    if (VIA_UNLIKELY(V->yield))
    {
        // This code is dogshit...
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
        Operand rdst = V->ip->operand1;
        Operand rsrc = V->ip->operand2;
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
            // Basically just LOAD lmao
            via_setregister(V, rdst.val_register, via_toviavalue(V, rsrc));

        VM_NEXT();
    }

    // Basically MOV but the registers aren't cleaned
    case OpCode::CPY:
    {
        Operand rdst = V->ip->operand1;
        Operand rsrc = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rdst), "Expected Register for CPY destination");
        VM_ASSERT(viaC_checkregister(rsrc), "Expected Register for CPY source");
#endif
        // This has to be copied, otherwise it will remain a reference
        viaValue cpy = *via_getregister(V, rsrc.val_register);
        via_setregister(V, rdst.val_register, cpy);

        VM_NEXT();
    }

    case OpCode::LOAD:
    {
        Operand rdst = V->ip->operand1;
        Operand imm = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rdst), "Expected Register for LOAD destination");
#endif
        viaValue val = via_toviavalue(V, imm);
        via_setregister(V, rdst.val_register, val);
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
        Operand arg = V->ip->operand1;
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
        Operand dst = V->ip->operand1;
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
        Operand ret = V->ip->operand1;
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
        Operand dst = V->ip->operand1;
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
        Operand val = V->ip->operand1;
        Operand id = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkidentifier(id), "Expected identifier for SETLOCAL id");
#endif
        VarId id_t = viaT_hashstring(V, id.val_identifier);
        // Slow-path: loaded value is a register
        if (VIA_UNLIKELY(viaC_checkregister(val)))
            via_setvariable(V, id_t, *via_getregister(V, val.val_register));
        else
            via_setvariable(V, id_t, via_toviavalue(V, val));

        VM_NEXT();
    }

    case OpCode::LOADLOCAL:
    {
        Operand dst = V->ip->operand1;
        Operand id = V->ip->operand2;

        VarId id_t = viaT_hashstring(V, id.val_identifier);

        via_loadvariable(V, id_t, dst.val_register);
        VM_NEXT();
    }

    case OpCode::SETGLOBAL:
    {
        Operand val = V->ip->operand1;
        Operand id = V->ip->operand2;

        // Slow-path: loaded value is a register
        if (VIA_UNLIKELY(viaC_checkregister(val)))
            via_setglobal(V, viaT_hashstring(V, id.val_identifier), *via_getregister(V, val.val_register));
        else
            via_setglobal(V, viaT_hashstring(V, id.val_identifier), via_toviavalue(V, val));

        VM_NEXT();
    }

    case OpCode::LOADGLOBAL:
    {
        Operand dst = V->ip->operand1;
        Operand id = V->ip->operand2;

        via_loadglobal(V, viaT_hashstring(V, id.val_identifier), dst.val_register);

        VM_NEXT();
    }

    case OpCode::LOADVAR:
    {
        Operand dst = V->ip->operand1;
        Operand id = V->ip->operand2;

        VarId id_t = viaT_hashstring(V, id.val_identifier);
        viaValue val = viaT_stackvalue(V, ValueType::Nil);

        for (viaFunction *frame : *V->stack)
        {
            auto it = frame->locals.find(id_t);
            if (it != frame->locals.end())
            {
                val = it->second;
                break;
            }
        }

        via_setregister(V, dst.val_register, val);
        VM_NEXT();
    }

    case OpCode::ADD:
    {
        Operand rdst = V->ip->operand1;
        Operand rlhs = V->ip->operand2;
        Operand rrhs = V->ip->operand3;
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rdst), "Expected register for arithmetic destination");
#endif
        viaValue lhs, rhs;

        if (VIA_LIKELY(viaC_checkregister(rlhs)))
            lhs = *via_getregister(V, rlhs.val_register);
        else
            lhs = via_toviavalue(V, rlhs);

        if (VIA_UNLIKELY(viaC_checkregister(rrhs)))
            rhs = *via_getregister(V, rrhs.val_register);
        else
            rhs = via_toviavalue(V, rrhs);

        via_setregister(V, rdst.val_register, via_arith(V, lhs, rhs, OpCode::ADD));
        VM_NEXT();
    }
    case OpCode::SUB:
    {
        Operand rdst = V->ip->operand1;
        Operand rlhs = V->ip->operand2;
        Operand rrhs = V->ip->operand3;
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rdst), "Expected register for arithmetic destination");
#endif
        viaValue lhs, rhs;

        if (VIA_LIKELY(viaC_checkregister(rlhs)))
            lhs = *via_getregister(V, rlhs.val_register);
        else
            lhs = via_toviavalue(V, rlhs);

        if (VIA_UNLIKELY(viaC_checkregister(rrhs)))
            rhs = *via_getregister(V, rrhs.val_register);
        else
            rhs = via_toviavalue(V, rrhs);

        via_setregister(V, rdst.val_register, via_arith(V, lhs, rhs, OpCode::SUB));
        VM_NEXT();
    }
    case OpCode::MUL:
    {
        Operand rdst = V->ip->operand1;
        Operand rlhs = V->ip->operand2;
        Operand rrhs = V->ip->operand3;
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rdst), "Expected register for arithmetic destination");
#endif
        viaValue lhs, rhs;

        if (VIA_LIKELY(viaC_checkregister(rlhs)))
            lhs = *via_getregister(V, rlhs.val_register);
        else
            lhs = via_toviavalue(V, rlhs);

        if (VIA_UNLIKELY(viaC_checkregister(rrhs)))
            rhs = *via_getregister(V, rrhs.val_register);
        else
            rhs = via_toviavalue(V, rrhs);

        via_setregister(V, rdst.val_register, via_arith(V, lhs, rhs, OpCode::MUL));
        VM_NEXT();
    }
    case OpCode::DIV:
    {
        Operand rdst = V->ip->operand1;
        Operand rlhs = V->ip->operand2;
        Operand rrhs = V->ip->operand3;
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rdst), "Expected register for arithmetic destination");
#endif
        viaValue lhs, rhs;

        if (VIA_LIKELY(viaC_checkregister(rlhs)))
            lhs = *via_getregister(V, rlhs.val_register);
        else
            lhs = via_toviavalue(V, rlhs);

        if (VIA_UNLIKELY(viaC_checkregister(rrhs)))
            rhs = *via_getregister(V, rrhs.val_register);
        else
            rhs = via_toviavalue(V, rrhs);

        via_setregister(V, rdst.val_register, via_arith(V, lhs, rhs, OpCode::DIV));
        VM_NEXT();
    }
    case OpCode::POW:
    {
        Operand rdst = V->ip->operand1;
        Operand rlhs = V->ip->operand2;
        Operand rrhs = V->ip->operand3;
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rdst), "Expected register for arithmetic destination");
#endif
        viaValue lhs, rhs;

        if (VIA_LIKELY(viaC_checkregister(rlhs)))
            lhs = *via_getregister(V, rlhs.val_register);
        else
            lhs = via_toviavalue(V, rlhs);

        if (VIA_UNLIKELY(viaC_checkregister(rrhs)))
            rhs = *via_getregister(V, rrhs.val_register);
        else
            rhs = via_toviavalue(V, rrhs);

        via_setregister(V, rdst.val_register, via_arith(V, lhs, rhs, OpCode::POW));
        VM_NEXT();
    }
    case OpCode::MOD:
    {
        Operand rdst = V->ip->operand1;
        Operand rlhs = V->ip->operand2;
        Operand rrhs = V->ip->operand3;
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rdst), "Expected register for arithmetic destination");
#endif
        viaValue lhs, rhs;

        if (VIA_LIKELY(viaC_checkregister(rlhs)))
            lhs = *via_getregister(V, rlhs.val_register);
        else
            lhs = via_toviavalue(V, rlhs);

        if (VIA_UNLIKELY(viaC_checkregister(rrhs)))
            rhs = *via_getregister(V, rrhs.val_register);
        else
            rhs = via_toviavalue(V, rrhs);

        via_setregister(V, rdst.val_register, via_arith(V, lhs, rhs, OpCode::MOD));
        VM_NEXT();
    }

    case OpCode::IADD:
    {
        Operand rlhs = V->ip->operand1;
        Operand rrhs = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rlhs), "Expected register for inline arithmetic operation");
#endif
        viaValue rhs;

        if (VIA_UNLIKELY(viaC_checkregister(rrhs)))
            rhs = *via_getregister(V, rrhs.val_register);
        else
            rhs = via_toviavalue(V, rrhs);

        via_iarith(V, via_getregister(V, rlhs.val_register), rhs, OpCode::IADD);
        VM_NEXT();
    }
    case OpCode::ISUB:
    {
        Operand rlhs = V->ip->operand1;
        Operand rrhs = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rlhs), "Expected register for inline arithmetic operation");
#endif
        viaValue rhs;

        if (VIA_UNLIKELY(viaC_checkregister(rrhs)))
            rhs = *via_getregister(V, rrhs.val_register);
        else
            rhs = via_toviavalue(V, rrhs);

        via_iarith(V, via_getregister(V, rlhs.val_register), rhs, OpCode::ISUB);
        VM_NEXT();
    }
    case OpCode::IMUL:
    {
        Operand rlhs = V->ip->operand1;
        Operand rrhs = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rlhs), "Expected register for inline arithmetic operation");
#endif
        viaValue rhs;

        if (VIA_UNLIKELY(viaC_checkregister(rrhs)))
            rhs = *via_getregister(V, rrhs.val_register);
        else
            rhs = via_toviavalue(V, rrhs);

        via_iarith(V, via_getregister(V, rlhs.val_register), rhs, OpCode::IMUL);
        VM_NEXT();
    }
    case OpCode::IDIV:
    {
        Operand rlhs = V->ip->operand1;
        Operand rrhs = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rlhs), "Expected register for inline arithmetic operation");
#endif
        viaValue rhs;

        if (VIA_UNLIKELY(viaC_checkregister(rrhs)))
            rhs = *via_getregister(V, rrhs.val_register);
        else
            rhs = via_toviavalue(V, rrhs);

        via_iarith(V, via_getregister(V, rlhs.val_register), rhs, OpCode::IDIV);
        VM_NEXT();
    }
    case OpCode::IPOW:
    {
        Operand rlhs = V->ip->operand1;
        Operand rrhs = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rlhs), "Expected register for inline arithmetic operation");
#endif
        viaValue rhs;

        if (VIA_UNLIKELY(viaC_checkregister(rrhs)))
            rhs = *via_getregister(V, rrhs.val_register);
        else
            rhs = via_toviavalue(V, rrhs);

        via_iarith(V, via_getregister(V, rlhs.val_register), rhs, OpCode::IPOW);
        VM_NEXT();
    }
    case OpCode::IMOD:
    {
        Operand rlhs = V->ip->operand1;
        Operand rrhs = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(viaC_checkregister(rlhs), "Expected register for inline arithmetic operation");
#endif
        viaValue rhs;

        if (VIA_UNLIKELY(viaC_checkregister(rrhs)))
            rhs = *via_getregister(V, rrhs.val_register);
        else
            rhs = via_toviavalue(V, rrhs);

        via_iarith(V, via_getregister(V, rlhs.val_register), rhs, OpCode::IMOD);
        VM_NEXT();
    }

    case OpCode::NEG:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;

        viaValue lhsn = *via_getregister(V, lhs.val_register);
#ifdef VIA_DEBUG
        VM_ASSERT(viaT_checknumber(V, lhsn), "Expected Number for binary operand 0");
#endif
        via_setregister(V, dst.val_register, viaT_stackvalue(V, -lhsn.val_number));
        VM_NEXT();
    }

    case OpCode::BAND:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        viaValue lhsn, rhsn;

        if (VIA_LIKELY(lhs.type == OperandType::Bool))
            lhsn.val_boolean = lhs.val_boolean;
        else
            lhsn = *via_getregister(V, lhs.val_register);

        if (VIA_LIKELY(rhs.type == OperandType::Bool))
            rhsn.val_boolean = rhs.val_boolean;
        else
            rhsn = *via_getregister(V, rhs.val_register);

        bool result = via_tobool(V, lhsn).val_boolean && via_tobool(V, rhsn).val_boolean;
        via_setregister(V, dst.val_register, viaT_stackvalue(V, result));
        VM_NEXT();
    }
    case OpCode::BOR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        viaValue lhsn, rhsn;

        if (VIA_LIKELY(lhs.type == OperandType::Bool))
            lhsn.val_boolean = lhs.val_boolean;
        else
            lhsn = *via_getregister(V, lhs.val_register);

        if (VIA_LIKELY(rhs.type == OperandType::Bool))
            rhsn.val_boolean = rhs.val_boolean;
        else
            rhsn = *via_getregister(V, rhs.val_register);

        bool result = via_tobool(V, lhsn).val_boolean || via_tobool(V, rhsn).val_boolean;
        via_setregister(V, dst.val_register, viaT_stackvalue(V, result));
        VM_NEXT();
    }
    case OpCode::BXOR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        viaValue lhsn, rhsn;

        if (VIA_LIKELY(lhs.type == OperandType::Bool))
            lhsn.val_boolean = lhs.val_boolean;
        else
            lhsn = *via_getregister(V, lhs.val_register);

        if (VIA_LIKELY(rhs.type == OperandType::Bool))
            rhsn.val_boolean = rhs.val_boolean;
        else
            rhsn = *via_getregister(V, rhs.val_register);

        bool result = via_tobool(V, lhsn).val_boolean != via_tobool(V, rhsn).val_boolean;
        via_setregister(V, dst.val_register, viaT_stackvalue(V, result));
        VM_NEXT();
    }

    case OpCode::BNOT:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        viaValue lhsn = *via_getregister(V, lhs.val_register);
#ifdef VIA_DEBUG
        VM_ASSERT(viaT_checkbool(V, lhsn), "Expected Bool for logical operand 0");
#endif
        via_setregister(V, dst.val_register, viaT_stackvalue(V, !lhsn.val_boolean));

        VM_NEXT();
    }

    case OpCode::EQ:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        viaValue lhsn, rhsn;

        bool lhs_reg = false;
        bool rhs_reg = false;

        if (VIA_UNLIKELY(lhs.type == OperandType::Register))
        {
            lhsn = *via_getregister(V, lhs.val_register);
            lhs_reg = true;
        }
        else
            lhsn = via_toviavalue(V, lhs);

        if (VIA_UNLIKELY(rhs.type == OperandType::Register))
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
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        viaValue lhsn, rhsn;

        bool lhs_reg = false;
        bool rhs_reg = false;

        if (VIA_UNLIKELY(lhs.type == OperandType::Register))
        {
            lhsn = *via_getregister(V, lhs.val_register);
            lhs_reg = true;
        }
        else
            lhsn = via_toviavalue(V, lhs);

        if (VIA_UNLIKELY(rhs.type == OperandType::Register))
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
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;
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
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;
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
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;
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
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;
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
        Operand rsrc = V->ip->operand1;

        viaValue *src = via_getregister(V, rsrc.val_register);
        std::cout << via_tostring(V, *src).val_string->ptr;

        VM_NEXT();
    }

    case OpCode::HALT:
    {
        via_setexitdata(V, 0, "VM halted by user");
        VM_EXIT();
    }

    case OpCode::EXIT:
    {
        Operand rcode = V->ip->operand1;

        viaValue *code = via_getregister(V, rcode.val_register);
        viaNumber ecode = via_tonumber(V, *code).val_number;

        via_setexitdata(V, static_cast<ExitCode>(ecode), "VM exited by user");
        VM_EXIT();
    }

    case OpCode::JMP:
    {
        Operand offset = V->ip->operand1;
#ifdef VIA_DEBUG
        VIA_ASSERT(viaC_checknumber(offset), "Expected number of JMP offset");
#endif
        V->ip += static_cast<JmpOffset>(offset.val_number);
        VM_NEXT();
    }

    case OpCode::JMPNZ:
    case OpCode::JMPZ:
    {
        Operand condr = V->ip->operand1;
        Operand offset = V->ip->operand2;

        viaValue cond = *via_getregister(V, condr.val_register);
        // We don't need to save the return value because this function modifies `cond`
        via_tonumber(V, cond);
        if (V->ip->op == OpCode::JMPNZ ? (cond.val_number != 0) : (cond.val_number == 0))
            V->ip += static_cast<JmpOffset>(offset.val_number);

        VM_NEXT();
    }

    case OpCode::JMPEQ:
    case OpCode::JMPNEQ:
    {
        Operand condlr = V->ip->operand1;
        Operand condrr = V->ip->operand2;
        Operand offset = V->ip->operand3;

        bool cond = via_cmpregister(V, condlr.val_register, condrr.val_register);
        if (V->ip->op == OpCode::JMPEQ ? cond : !cond)
            V->ip += static_cast<JmpOffset>(offset.val_number);

        VM_NEXT();
    }

    case OpCode::JMPLT:
    {
        Operand condlr = V->ip->operand1;
        Operand condrr = V->ip->operand2;
        Operand offset = V->ip->operand3;

        viaValue *lhs = via_getregister(V, condlr.val_register);
        viaValue *rhs = via_getregister(V, condrr.val_register);

        bool cond = lhs->val_number < rhs->val_number;
        if (cond)
            V->ip += static_cast<JmpOffset>(offset.val_number);

        VM_NEXT();
    }

    case OpCode::JMPGT:
    {
        Operand condlr = V->ip->operand1;
        Operand condrr = V->ip->operand2;
        Operand offset = V->ip->operand3;

        viaValue *lhs = via_getregister(V, condlr.val_register);
        viaValue *rhs = via_getregister(V, condrr.val_register);

        bool cond = lhs->val_number > rhs->val_number;
        if (cond)
            V->ip += static_cast<JmpOffset>(offset.val_number);

        VM_NEXT();
    }

    case OpCode::JMPLE:
    {
        Operand condlr = V->ip->operand1;
        Operand condrr = V->ip->operand2;
        Operand offset = V->ip->operand3;

        viaValue *lhs = via_getregister(V, condlr.val_register);
        viaValue *rhs = via_getregister(V, condrr.val_register);

        bool cond = lhs->val_number <= rhs->val_number;
        if (cond)
            V->ip += static_cast<JmpOffset>(offset.val_number);

        VM_NEXT();
    }

    case OpCode::JMPGE:
    {
        Operand condlr = V->ip->operand1;
        Operand condrr = V->ip->operand2;
        Operand offset = V->ip->operand3;

        viaValue *lhs = via_getregister(V, condlr.val_register);
        viaValue *rhs = via_getregister(V, condrr.val_register);

        bool cond = lhs->val_number >= rhs->val_number;
        if (cond)
            V->ip += static_cast<JmpOffset>(offset.val_number);

        VM_NEXT();
    }

    case OpCode::JMPLBL:
    {
        Operand label = V->ip->operand1;
        auto it = V->labels->find(std::string_view(label.val_identifier));
#ifdef VIA_DEBUG
        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));
#endif
        // +1 Because if we jump to the exact position of the label, the VM will interpret it as
        // a declaration, not a jump
        V->ip = it->second + 1;
        VM_NEXT();
    }

    case OpCode::JMPLBLZ:
    case OpCode::JMPLBLNZ:
    {
        Operand valr = V->ip->operand1;
        Operand label = V->ip->operand2;

        auto it = V->labels->find(std::string_view(label.val_identifier));
#ifdef VIA_DEBUG
        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));
#endif
        viaValue *val = via_getregister(V, valr.val_register);
        bool cond = val->val_number == 0;
        // Jump if the condition is met
        if (V->ip->op == OpCode::JMPLBLZ ? cond : !cond)
            V->ip = it->second + 1;

        VM_NEXT();
    }

    case OpCode::JMPLBLEQ:
    case OpCode::JMPLBLNEQ:
    {
        Operand lhsr = V->ip->operand1;
        Operand rhsr = V->ip->operand2;
        Operand label = V->ip->operand3;

        auto it = V->labels->find(LabelId(label.val_identifier));
#ifdef VIA_DEBUG
        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));
#endif
        bool cond = via_cmpregister(V, lhsr.val_register, rhsr.val_register);

        // Jump if the condition is met
        if (V->ip->op == OpCode::JMPLBLEQ ? cond : !cond)
            V->ip = it->second + 1;

        VM_NEXT();
    }

    case OpCode::JMPLBLLT:
    {
        Operand lhsr = V->ip->operand1;
        Operand rhsr = V->ip->operand2;
        Operand label = V->ip->operand3;

        auto it = V->labels->find(std::string_view(label.val_identifier));
#ifdef VIA_DEBUG
        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));
#endif
        bool cond = via_getregister(V, lhsr.val_register)->val_number < via_getregister(V, rhsr.val_register)->val_number;

        // Jump if the condition is met
        if (cond)
            V->ip = it->second + 1;

        VM_NEXT();
    }

    case OpCode::JMPLBLGT:
    {
        Operand lhsr = V->ip->operand1;
        Operand rhsr = V->ip->operand2;
        Operand label = V->ip->operand3;

        auto it = V->labels->find(std::string_view(label.val_identifier));
#ifdef VIA_DEBUG
        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));
#endif
        bool cond = via_getregister(V, lhsr.val_register)->val_number > via_getregister(V, rhsr.val_register)->val_number;
        // Jump if the condition is met
        if (cond)
            V->ip = it->second + 1;

        VM_NEXT();
    }

    case OpCode::JMPLBLLE:
    {
        Operand lhsr = V->ip->operand1;
        Operand rhsr = V->ip->operand2;
        Operand label = V->ip->operand3;

        auto it = V->labels->find(std::string_view(label.val_identifier));
#ifdef VIA_DEBUG
        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));
#endif
        bool cond = via_getregister(V, lhsr.val_register)->val_number <= via_getregister(V, rhsr.val_register)->val_number;

        // Jump if the condition is met
        if (cond)
            V->ip = it->second + 1;

        VM_NEXT();
    }

    case OpCode::JMPLBLGE:
    {
        Operand lhsr = V->ip->operand1;
        Operand rhsr = V->ip->operand2;
        Operand label = V->ip->operand3;

        auto it = V->labels->find(std::string_view(label.val_identifier));
#ifdef VIA_DEBUG
        VM_ASSERT(it != V->labels->end(), std::format("Label '{}' not found", label.val_identifier));
#endif
        bool cond = via_getregister(V, lhsr.val_register)->val_number >= via_getregister(V, rhsr.val_register)->val_number;

        // Jump if the condition is met
        if (cond)
            V->ip = it->second + 1;

        VM_NEXT();
    }

    case OpCode::CALL:
    {
        Operand rfn = V->ip->operand1;
        Operand argc = V->ip->operand2;

        // Set argc
        V->argc = static_cast<CallArgc>(argc.val_number);

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
        Operand id = V->ip->operand1;
        LabelId key = id.val_identifier;
        Instruction *instr = V->ip;

        auto it = V->labels->find(key);
        if (it == V->labels->end())
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
        Operand rfn = V->ip->operand1;
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
            Instruction cpy = *V->ip;
            fn->bytecode.push_back(cpy);

            // Mark OpCode as NOP so the VM skips this instruction
            V->ip->op = OpCode::NOP;
            V->ip++;
        }

        // Dispatch instead of invoking VM_NEXT
        VM_DISPATCH();
    }

    case OpCode::LOADIDX:
    {
        Operand rdst = V->ip->operand1;
        Operand rtbl = V->ip->operand2;
        Operand ridx = V->ip->operand3;

        viaValue tbl = *via_getregister(V, rtbl.val_register);
        viaValue idx = *via_getregister(V, ridx.val_register);

        // Get table key based on the index type (string or number)
        TableKey key = viaT_checkstring(V, idx) ? idx.val_string->hash : idx.val_number;
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
        Operand rsrc = V->ip->operand1;
        Operand rtbl = V->ip->operand2;
        Operand ridx = V->ip->operand3;

        viaValue val;
        viaValue tbl = *via_getregister(V, rtbl.val_register);
        viaValue idx = *via_getregister(V, ridx.val_register);

        // Get table key based on the index type (string or number)
        TableKey key = viaT_checkstring(V, idx) ? idx.val_string->hash : static_cast<Hash>(idx.val_number);
#ifdef VIA_DEBUG
        // Assert that the value is a table
        std::string _errfmt = std::format("Attempt to assign index to {}", ENUM_NAME(tbl.type));
        VM_ASSERT(viaT_checktable(V, tbl), _errfmt);
#endif
        // Slow-path: the value is stored in a register, load it
        if (VIA_UNLIKELY(rsrc.type == OperandType::Register))
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
        Operand rdst = V->ip->operand1;
        Operand objr = V->ip->operand2;

        viaValue val;

        // Fast-path: object is stored in a register
        if (VIA_LIKELY(objr.type == OperandType::Register))
            val = *via_getregister(V, objr.val_register);
        else
            val = via_toviavalue(V, objr);

        via_setregister(V, rdst.val_register, via_len(V, val));

        VM_NEXT();
    }

    case OpCode::TYPE:
    {
        Operand rdst = V->ip->operand1;
        Operand objr = V->ip->operand2;

        viaValue *val = via_getregister(V, objr.val_register);
        viaValue type = via_type(V, *val);

        via_setregister(V, rdst.val_register, type);

        VM_NEXT();
    }

    case OpCode::TYPEOF:
    {
        Operand rdst = V->ip->operand1;
        Operand objr = V->ip->operand2;

        viaValue *val = via_getregister(V, objr.val_register);
        viaValue type = via_typeof(V, *val);

        via_setregister(V, rdst.val_register, type);

        VM_NEXT();
    }

    case OpCode::STRCON:
    {
        Operand rdst = V->ip->operand1;
        Operand lhsr = V->ip->operand2;
        Operand rhsr = V->ip->operand3;

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

    default:
        via_setexitdata(V, 1, std::format("Unrecognized OpCode (op_id={})", static_cast<uint8_t>(V->ip->op)));
        VM_EXIT();
    }
}

exit:;
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