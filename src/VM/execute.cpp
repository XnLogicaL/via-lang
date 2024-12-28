/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "execute.h"
#include "api.h"
#include "chunk.h"
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
        if (!isvalidjmpaddr(V, V->ip + 1)) \
        { \
            setexitdata(V, 0, ""); \
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
            setexitdata(V, 1, std::format("VM_ASSERT(): {}\n in file {}, line {}", (message), __FILE__, __LINE__).c_str()); \
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
inline void _optimize_empty_instruction_sequence(RTState *V)
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
void execute(RTState *V)
{
    VIA_ASSERT(V->tstate != ThreadState::RUNNING, "execute() called on running thread (tstate=RUNNING)");
    VIA_ASSERT(V->tstate != ThreadState::DEAD, "execute() called on dead thread (tstate=DEAD)");
    V->tstate = ThreadState::RUNNING;

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
        *V = *V->sstate;
        V->sstate = nullptr;
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
        VIA_UNLIKELY(isvalidjmpaddr(V, V->ip)),
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
        long long milliseconds = V->yieldfor / 1000;
        std::chrono::milliseconds dur(milliseconds);
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
        VM_ASSERT(ccheckregister(rsrc), "Expected GPRegister for MOV source");
#endif
        // Fast-path: both operands are registers
        if (VIA_LIKELY(ccheckregister(rdst)))
        {
            setregister(V, rdst.val_register, *getregister(V, rsrc.val_register));
            // Set the <src> register to nil
            setregister(V, rdst.val_register, stackvalue(V));
        }
        else
            // Basically just LOAD lmao
            setregister(V, rdst.val_register, toviavalue(V, rsrc));

        VM_NEXT();
    }

    // Basically MOV but the registers aren't cleaned
    case OpCode::CPY:
    {
        Operand rdst = V->ip->operand1;
        Operand rsrc = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(rdst), "Expected GPRegister for CPY destination");
        VM_ASSERT(ccheckregister(rsrc), "Expected GPRegister for CPY source");
#endif
        // This has to be copied, otherwise it will remain a reference
        TValue cpy = *getregister(V, rsrc.val_register);
        setregister(V, rdst.val_register, cpy);

        VM_NEXT();
    }

    case OpCode::LOAD:
    {
        Operand rdst = V->ip->operand1;
        Operand imm = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(rdst), "Expected GPRegister for LOAD destination");
#endif
        TValue val = toviavalue(V, imm);
        setregister(V, rdst.val_register, val);
        VM_NEXT();
    }

    case OpCode::PUSH:
    {
        TFunction *frame = new TFunction{
            0,
            false,
            false,
            "LC",
            tstop(V->stack),
            {},
            {},
        };

        tspush(V->stack, frame);
        VM_NEXT();
    }

    case OpCode::POP:
    {
#ifdef VIA_DEBUG
        if (V->stack->size >= 1)
        {
            setexitdata(V, 1, "Illegal pop: restricted stack frame");
            VM_EXIT();
        }
#endif
        tspop(V->stack);
        VM_NEXT();
    }

    case OpCode::PUSHARG:
    {
        Operand arg = V->ip->operand1;
        TValue arg_val;

        if (VIA_LIKELY(ccheckregister(arg)))
            arg_val = *getregister(V, arg.val_register);
        else
            arg_val = toviavalue(V, arg);

        tspush(V->arguments, arg_val);
        VM_NEXT();
    }

    case OpCode::POPARG:
    {
        Operand dst = V->ip->operand1;
        TValue val = tstop(V->arguments);
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(dst), "Expected register for POPARG destination");
#endif
        tspop(V->arguments);
        setregister(V, dst.val_register, val);
        VM_NEXT();
    }

    case OpCode::PUSHRET:
    {
        Operand ret = V->ip->operand1;
        TValue ret_val;

        if (VIA_LIKELY(ccheckregister(ret)))
            ret_val = *getregister(V, ret.val_register);
        else
            ret_val = toviavalue(V, ret);

        tspush(V->returns, ret_val);
        VM_NEXT();
    }

    case OpCode::POPRET:
    {
        Operand dst = V->ip->operand1;
        TValue val = tstop(V->returns);
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(dst), "Expected register for POPRET destination");
#endif
        tspop(V->returns);
        setregister(V, dst.val_register, val);
        VM_NEXT();
    }

    case OpCode::SETLOCAL:
    {
        Operand val = V->ip->operand1;
        Operand id = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckidentifier(id), "Expected identifier for SETLOCAL id");
#endif
        VarId id_t = hashstring(V, id.val_identifier);
        // Slow-path: loaded value is a register
        if (VIA_UNLIKELY(ccheckregister(val)))
            setvariable(V, id_t, *getregister(V, val.val_register));
        else
            setvariable(V, id_t, toviavalue(V, val));

        VM_NEXT();
    }

    case OpCode::LOADLOCAL:
    {
        Operand dst = V->ip->operand1;
        Operand id = V->ip->operand2;

        VarId id_t = hashstring(V, id.val_identifier);

        loadvariable(V, id_t, dst.val_register);
        VM_NEXT();
    }

    case OpCode::SETGLOBAL:
    {
        Operand val = V->ip->operand1;
        Operand id = V->ip->operand2;

        // Slow-path: loaded value is a register
        if (VIA_UNLIKELY(ccheckregister(val)))
            setglobal(V, hashstring(V, id.val_identifier), *getregister(V, val.val_register));
        else
            setglobal(V, hashstring(V, id.val_identifier), toviavalue(V, val));

        VM_NEXT();
    }

    case OpCode::LOADGLOBAL:
    {
        Operand dst = V->ip->operand1;
        Operand id = V->ip->operand2;

        loadglobal(V, hashstring(V, id.val_identifier), dst.val_register);
        VM_NEXT();
    }

    case OpCode::LOADVAR:
    {
        Operand dst = V->ip->operand1;
        Operand id = V->ip->operand2;

        VarId id_t = hashstring(V, id.val_identifier);
        TValue val = stackvalue(V, ValueType::Nil);

        for (TFunction *frame : *V->stack)
        {
            auto it = frame->locals.find(id_t);
            if (it != frame->locals.end())
            {
                val = it->second;
                break;
            }
        }

        setregister(V, dst.val_register, val);
        VM_NEXT();
    }

    case OpCode::ADD:
    {
        Operand rdst = V->ip->operand1;
        Operand rlhs = V->ip->operand2;
        Operand rrhs = V->ip->operand3;
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(rdst), "Expected register for arithmetic destination");
#endif
        TValue lhs, rhs;

        if (VIA_LIKELY(ccheckregister(rlhs)))
            lhs = *getregister(V, rlhs.val_register);
        else
            lhs = toviavalue(V, rlhs);

        if (VIA_UNLIKELY(ccheckregister(rrhs)))
            rhs = *getregister(V, rrhs.val_register);
        else
            rhs = toviavalue(V, rrhs);

        setregister(V, rdst.val_register, arith(V, lhs, rhs, OpCode::ADD));
        VM_NEXT();
    }
    case OpCode::SUB:
    {
        Operand rdst = V->ip->operand1;
        Operand rlhs = V->ip->operand2;
        Operand rrhs = V->ip->operand3;
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(rdst), "Expected register for arithmetic destination");
#endif
        TValue lhs, rhs;

        if (VIA_LIKELY(ccheckregister(rlhs)))
            lhs = *getregister(V, rlhs.val_register);
        else
            lhs = toviavalue(V, rlhs);

        if (VIA_UNLIKELY(ccheckregister(rrhs)))
            rhs = *getregister(V, rrhs.val_register);
        else
            rhs = toviavalue(V, rrhs);

        setregister(V, rdst.val_register, arith(V, lhs, rhs, OpCode::SUB));
        VM_NEXT();
    }
    case OpCode::MUL:
    {
        Operand rdst = V->ip->operand1;
        Operand rlhs = V->ip->operand2;
        Operand rrhs = V->ip->operand3;
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(rdst), "Expected register for arithmetic destination");
#endif
        TValue lhs, rhs;

        if (VIA_LIKELY(ccheckregister(rlhs)))
            lhs = *getregister(V, rlhs.val_register);
        else
            lhs = toviavalue(V, rlhs);

        if (VIA_UNLIKELY(ccheckregister(rrhs)))
            rhs = *getregister(V, rrhs.val_register);
        else
            rhs = toviavalue(V, rrhs);

        setregister(V, rdst.val_register, arith(V, lhs, rhs, OpCode::MUL));
        VM_NEXT();
    }
    case OpCode::DIV:
    {
        Operand rdst = V->ip->operand1;
        Operand rlhs = V->ip->operand2;
        Operand rrhs = V->ip->operand3;
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(rdst), "Expected register for arithmetic destination");
#endif
        TValue lhs, rhs;

        if (VIA_LIKELY(ccheckregister(rlhs)))
            lhs = *getregister(V, rlhs.val_register);
        else
            lhs = toviavalue(V, rlhs);

        if (VIA_UNLIKELY(ccheckregister(rrhs)))
            rhs = *getregister(V, rrhs.val_register);
        else
            rhs = toviavalue(V, rrhs);

        setregister(V, rdst.val_register, arith(V, lhs, rhs, OpCode::DIV));
        VM_NEXT();
    }
    case OpCode::POW:
    {
        Operand rdst = V->ip->operand1;
        Operand rlhs = V->ip->operand2;
        Operand rrhs = V->ip->operand3;
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(rdst), "Expected register for arithmetic destination");
#endif
        TValue lhs, rhs;

        if (VIA_LIKELY(ccheckregister(rlhs)))
            lhs = *getregister(V, rlhs.val_register);
        else
            lhs = toviavalue(V, rlhs);

        if (VIA_UNLIKELY(ccheckregister(rrhs)))
            rhs = *getregister(V, rrhs.val_register);
        else
            rhs = toviavalue(V, rrhs);

        setregister(V, rdst.val_register, arith(V, lhs, rhs, OpCode::POW));
        VM_NEXT();
    }
    case OpCode::MOD:
    {
        Operand rdst = V->ip->operand1;
        Operand rlhs = V->ip->operand2;
        Operand rrhs = V->ip->operand3;
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(rdst), "Expected register for arithmetic destination");
#endif
        TValue lhs, rhs;

        if (VIA_LIKELY(ccheckregister(rlhs)))
            lhs = *getregister(V, rlhs.val_register);
        else
            lhs = toviavalue(V, rlhs);

        if (VIA_UNLIKELY(ccheckregister(rrhs)))
            rhs = *getregister(V, rrhs.val_register);
        else
            rhs = toviavalue(V, rrhs);

        setregister(V, rdst.val_register, arith(V, lhs, rhs, OpCode::MOD));
        VM_NEXT();
    }

    case OpCode::IADD:
    {
        Operand rlhs = V->ip->operand1;
        Operand rrhs = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(rlhs), "Expected register for inline arithmetic operation");
#endif
        TValue rhs;

        if (VIA_UNLIKELY(ccheckregister(rrhs)))
            rhs = *getregister(V, rrhs.val_register);
        else
            rhs = toviavalue(V, rrhs);

        iarith(V, getregister(V, rlhs.val_register), rhs, OpCode::IADD);
        VM_NEXT();
    }
    case OpCode::ISUB:
    {
        Operand rlhs = V->ip->operand1;
        Operand rrhs = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(rlhs), "Expected register for inline arithmetic operation");
#endif
        TValue rhs;

        if (VIA_UNLIKELY(ccheckregister(rrhs)))
            rhs = *getregister(V, rrhs.val_register);
        else
            rhs = toviavalue(V, rrhs);

        iarith(V, getregister(V, rlhs.val_register), rhs, OpCode::ISUB);
        VM_NEXT();
    }
    case OpCode::IMUL:
    {
        Operand rlhs = V->ip->operand1;
        Operand rrhs = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(rlhs), "Expected register for inline arithmetic operation");
#endif
        TValue rhs;

        if (VIA_UNLIKELY(ccheckregister(rrhs)))
            rhs = *getregister(V, rrhs.val_register);
        else
            rhs = toviavalue(V, rrhs);

        iarith(V, getregister(V, rlhs.val_register), rhs, OpCode::IMUL);
        VM_NEXT();
    }
    case OpCode::IDIV:
    {
        Operand rlhs = V->ip->operand1;
        Operand rrhs = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(rlhs), "Expected register for inline arithmetic operation");
#endif
        TValue rhs;

        if (VIA_UNLIKELY(ccheckregister(rrhs)))
            rhs = *getregister(V, rrhs.val_register);
        else
            rhs = toviavalue(V, rrhs);

        iarith(V, getregister(V, rlhs.val_register), rhs, OpCode::IDIV);
        VM_NEXT();
    }
    case OpCode::IPOW:
    {
        Operand rlhs = V->ip->operand1;
        Operand rrhs = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(rlhs), "Expected register for inline arithmetic operation");
#endif
        TValue rhs;

        if (VIA_UNLIKELY(ccheckregister(rrhs)))
            rhs = *getregister(V, rrhs.val_register);
        else
            rhs = toviavalue(V, rrhs);

        iarith(V, getregister(V, rlhs.val_register), rhs, OpCode::IPOW);
        VM_NEXT();
    }
    case OpCode::IMOD:
    {
        Operand rlhs = V->ip->operand1;
        Operand rrhs = V->ip->operand2;
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(rlhs), "Expected register for inline arithmetic operation");
#endif
        TValue rhs;

        if (VIA_UNLIKELY(ccheckregister(rrhs)))
            rhs = *getregister(V, rrhs.val_register);
        else
            rhs = toviavalue(V, rrhs);

        iarith(V, getregister(V, rlhs.val_register), rhs, OpCode::IMOD);
        VM_NEXT();
    }

    case OpCode::NEG:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;

        TValue lhsn = *getregister(V, lhs.val_register);
#ifdef VIA_DEBUG
        VM_ASSERT(checknumber(V, lhsn), "Expected Number for binary operand 0");
#endif
        setregister(V, dst.val_register, stackvalue(V, -lhsn.val_number));
        VM_NEXT();
    }

    case OpCode::BAND:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsn, rhsn;

        if (VIA_LIKELY(lhs.type == OperandType::Bool))
            lhsn.val_boolean = lhs.val_boolean;
        else
            lhsn = *getregister(V, lhs.val_register);

        if (VIA_LIKELY(rhs.type == OperandType::Bool))
            rhsn.val_boolean = rhs.val_boolean;
        else
            rhsn = *getregister(V, rhs.val_register);

        bool result = tobool(V, lhsn).val_boolean && tobool(V, rhsn).val_boolean;
        setregister(V, dst.val_register, stackvalue(V, result));
        VM_NEXT();
    }
    case OpCode::BOR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsn, rhsn;

        if (VIA_LIKELY(lhs.type == OperandType::Bool))
            lhsn.val_boolean = lhs.val_boolean;
        else
            lhsn = *getregister(V, lhs.val_register);

        if (VIA_LIKELY(rhs.type == OperandType::Bool))
            rhsn.val_boolean = rhs.val_boolean;
        else
            rhsn = *getregister(V, rhs.val_register);

        bool result = tobool(V, lhsn).val_boolean || tobool(V, rhsn).val_boolean;
        setregister(V, dst.val_register, stackvalue(V, result));
        VM_NEXT();
    }
    case OpCode::BXOR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsn, rhsn;

        if (VIA_LIKELY(lhs.type == OperandType::Bool))
            lhsn.val_boolean = lhs.val_boolean;
        else
            lhsn = *getregister(V, lhs.val_register);

        if (VIA_LIKELY(rhs.type == OperandType::Bool))
            rhsn.val_boolean = rhs.val_boolean;
        else
            rhsn = *getregister(V, rhs.val_register);

        bool result = tobool(V, lhsn).val_boolean != tobool(V, rhsn).val_boolean;
        setregister(V, dst.val_register, stackvalue(V, result));
        VM_NEXT();
    }

    case OpCode::BNOT:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        TValue lhsn = *getregister(V, lhs.val_register);
#ifdef VIA_DEBUG
        VM_ASSERT(checkbool(V, lhsn), "Expected Bool for logical operand 0");
#endif
        setregister(V, dst.val_register, stackvalue(V, !lhsn.val_boolean));

        VM_NEXT();
    }

    case OpCode::EQ:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsn, rhsn;

        bool lhs_reg = false;
        bool rhs_reg = false;

        if (VIA_UNLIKELY(lhs.type == OperandType::GPRegister))
        {
            lhsn = *getregister(V, lhs.val_register);
            lhs_reg = true;
        }
        else
            lhsn = toviavalue(V, lhs);

        if (VIA_UNLIKELY(rhs.type == OperandType::GPRegister))
        {
            rhsn = *getregister(V, rhs.val_register);
            rhs_reg = true;
        }
        else
            rhsn = toviavalue(V, rhs);

        if (lhs_reg && rhs_reg)
            setregister(V, dst.val_register, stackvalue(V, cmpregister(V, lhs.val_register, rhs.val_register)));
        else if (lhs_reg)
            setregister(V, dst.val_register, stackvalue(V, compare(V, *getregister(V, lhs.val_register), rhsn)));
        else if (rhs_reg)
            setregister(V, dst.val_register, stackvalue(V, compare(V, *getregister(V, rhs.val_register), lhsn)));
        else
            setregister(V, dst.val_register, stackvalue(V, compare(V, lhsn, rhsn)));

        VM_NEXT();
    }
    case OpCode::NEQ:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsn, rhsn;

        bool lhs_reg = false;
        bool rhs_reg = false;

        if (VIA_UNLIKELY(lhs.type == OperandType::GPRegister))
        {
            lhsn = *getregister(V, lhs.val_register);
            lhs_reg = true;
        }
        else
            lhsn = toviavalue(V, lhs);

        if (VIA_UNLIKELY(rhs.type == OperandType::GPRegister))
        {
            rhsn = *getregister(V, rhs.val_register);
            rhs_reg = true;
        }
        else
            rhsn = toviavalue(V, rhs);

        if (lhs_reg && rhs_reg)
            setregister(V, dst.val_register, stackvalue(V, !cmpregister(V, lhs.val_register, rhs.val_register)));
        else if (lhs_reg)
            setregister(V, dst.val_register, stackvalue(V, !compare(V, *getregister(V, lhs.val_register), rhsn)));
        else if (rhs_reg)
            setregister(V, dst.val_register, stackvalue(V, !compare(V, *getregister(V, rhs.val_register), lhsn)));
        else
            setregister(V, dst.val_register, stackvalue(V, !compare(V, lhsn, rhsn)));

        VM_NEXT();
    }
    case OpCode::LT:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;
#ifdef VIA_DEBUG
        VM_ASSERT(ccheckregister(dst), "Expected register for LT destination");
        VM_ASSERT(ccheckregister(lhs), "Expected register for LT lhs");
        VM_ASSERT(ccheckregister(rhs), "Expected register for LT rhs");
#endif
        TValue lhsn = *getregister(V, lhs.val_register);
        TValue rhsn = *getregister(V, rhs.val_register);
#ifdef VIA_DEBUG
        VM_ASSERT(checknumber(V, rhsn), "Expected Number for LT rvalue");
#endif
        if (VIA_LIKELY(checknumber(V, lhsn)))
        {
            setregister(V, dst.val_register, stackvalue(V, lhsn.val_number < rhsn.val_number));
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(checktable(V, lhsn)))
        {
            TValue *mm = getmetamethod(V, lhsn, OpCode::LT);
#ifdef VIA_DEBUG
            VM_ASSERT(checkcallable(V, *mm), "Expected callable metamethod for LT lvalue");
#endif
            pushargument(V, rhsn);
            call(V, *mm);
            setregister(V, dst.val_register, popargument(V));
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
        VM_ASSERT(ccheckregister(dst), "Expected register for GT destination");
        VM_ASSERT(ccheckregister(lhs), "Expected register for GT lhs");
        VM_ASSERT(ccheckregister(rhs), "Expected register for GT rhs");
#endif
        TValue lhsn = *getregister(V, lhs.val_register);
        TValue rhsn = *getregister(V, rhs.val_register);
#ifdef VIA_DEBUG
        VM_ASSERT(checknumber(V, rhsn), "Expected Number for GT rvalue");
#endif
        if (VIA_LIKELY(checknumber(V, lhsn)))
        {
            setregister(V, dst.val_register, stackvalue(V, lhsn.val_number > rhsn.val_number));
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(checktable(V, lhsn)))
        {
            TValue *mm = getmetamethod(V, lhsn, OpCode::GT);
#ifdef VIA_DEBUG
            VM_ASSERT(checkcallable(V, *mm), "Expected callable metamethod for GT lvalue");
#endif
            pushargument(V, rhsn);
            call(V, *mm);
            setregister(V, dst.val_register, popargument(V));
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
        VM_ASSERT(ccheckregister(dst), "Expected register for LE destination");
        VM_ASSERT(ccheckregister(lhs), "Expected register for LE lhs");
        VM_ASSERT(ccheckregister(rhs), "Expected register for LE rhs");
#endif
        TValue lhsn = *getregister(V, lhs.val_register);
        TValue rhsn = *getregister(V, rhs.val_register);
#ifdef VIA_DEBUG
        VM_ASSERT(checknumber(V, rhsn), "Expected Number for LE rvalue");
#endif
        if (VIA_LIKELY(checknumber(V, lhsn)))
        {
            setregister(V, dst.val_register, stackvalue(V, lhsn.val_number <= rhsn.val_number));
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(checktable(V, lhsn)))
        {
            TValue *mm = getmetamethod(V, lhsn, OpCode::LE);
#ifdef VIA_DEBUG
            VM_ASSERT(checkcallable(V, *mm), "Expected callable metamethod for LE lvalue");
#endif
            pushargument(V, rhsn);
            call(V, *mm);
            setregister(V, dst.val_register, popargument(V));
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
        VM_ASSERT(ccheckregister(dst), "Expected register for GE destination");
        VM_ASSERT(ccheckregister(lhs), "Expected register for GE lhs");
        VM_ASSERT(ccheckregister(rhs), "Expected register for GE rhs");
#endif
        TValue lhsn = *getregister(V, lhs.val_register);
        TValue rhsn = *getregister(V, rhs.val_register);
#ifdef VIA_DEBUG
        VM_ASSERT(checknumber(V, rhsn), "Expected Number for GE rvalue");
#endif
        if (VIA_LIKELY(checknumber(V, lhsn)))
        {
            setregister(V, dst.val_register, stackvalue(V, lhsn.val_number >= rhsn.val_number));
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(checktable(V, lhsn)))
        {
            TValue *mm = getmetamethod(V, lhsn, OpCode::GE);
#ifdef VIA_DEBUG
            VM_ASSERT(checkcallable(V, *mm), "Expected callable metamethod for GE lvalue");
#endif
            pushargument(V, rhsn);
            call(V, *mm);
            setregister(V, dst.val_register, popargument(V));
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

        TValue *src = getregister(V, rsrc.val_register);
        std::cout << tostring(V, *src).val_string->ptr;

        VM_NEXT();
    }

    case OpCode::HALT:
    {
        setexitdata(V, 0, "VM halted by user");
        VM_EXIT();
    }

    case OpCode::EXIT:
    {
        Operand rcode = V->ip->operand1;

        TValue *code = getregister(V, rcode.val_register);
        TNumber ecode = tonumber(V, *code).val_number;

        setexitdata(V, static_cast<ExitCode>(ecode), "VM exited by user");
        VM_EXIT();
    }

    case OpCode::JMP:
    {
        Operand offset = V->ip->operand1;
#ifdef VIA_DEBUG
        VIA_ASSERT(cchecknumber(offset), "Expected number of JMP offset");
#endif
        V->ip += static_cast<JmpOffset>(offset.val_number);
        VM_NEXT();
    }

    case OpCode::JMPNZ:
    case OpCode::JMPZ:
    {
        Operand condr = V->ip->operand1;
        Operand offset = V->ip->operand2;

        TValue cond = *getregister(V, condr.val_register);
        // We don't need to save the return value because this function modifies `cond`
        tonumber(V, cond);
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

        bool cond = cmpregister(V, condlr.val_register, condrr.val_register);
        if (V->ip->op == OpCode::JMPEQ ? cond : !cond)
            V->ip += static_cast<JmpOffset>(offset.val_number);

        VM_NEXT();
    }

    case OpCode::JMPLT:
    {
        Operand condlr = V->ip->operand1;
        Operand condrr = V->ip->operand2;
        Operand offset = V->ip->operand3;

        TValue *lhs = getregister(V, condlr.val_register);
        TValue *rhs = getregister(V, condrr.val_register);

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

        TValue *lhs = getregister(V, condlr.val_register);
        TValue *rhs = getregister(V, condrr.val_register);

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

        TValue *lhs = getregister(V, condlr.val_register);
        TValue *rhs = getregister(V, condrr.val_register);

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

        TValue *lhs = getregister(V, condlr.val_register);
        TValue *rhs = getregister(V, condrr.val_register);

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
        TValue *val = getregister(V, valr.val_register);
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
        bool cond = cmpregister(V, lhsr.val_register, rhsr.val_register);

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
        bool cond = getregister(V, lhsr.val_register)->val_number < getregister(V, rhsr.val_register)->val_number;

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
        bool cond = getregister(V, lhsr.val_register)->val_number > getregister(V, rhsr.val_register)->val_number;
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
        bool cond = getregister(V, lhsr.val_register)->val_number <= getregister(V, rhsr.val_register)->val_number;

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
        bool cond = getregister(V, lhsr.val_register)->val_number >= getregister(V, rhsr.val_register)->val_number;

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
        call(V, *getregister(V, rfn.val_register));
        VM_NEXT();
    }

    case OpCode::RET:
    {
        tspop(V->stack);
        restorestate(V);

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
        TFunction *fn = new TFunction;
        fn->line = 0;
        // TODO: Accept a valid user-set ID
        fn->id = "<anonymous-function>";
        fn->locals = {};

        setregister(V, rfn.val_register, stackvalue(V, fn));

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

        TValue tbl = *getregister(V, rtbl.val_register);
        TValue idx = *getregister(V, ridx.val_register);

        // Get table key based on the index type (string or number)
        TableKey key = checkstring(V, idx) ? idx.val_string->hash : idx.val_number;
#ifdef VIA_DEBUG
        // Assert that the value is a table
        std::string _errfmt = std::format("Attempt to load index of {}", type(V, tbl).val_string->ptr);
        VM_ASSERT(checktable(V, tbl), _errfmt);
#endif
        // Load the table index
        loadtableindex(V, tbl.val_table, key, rdst.val_register);
        VM_NEXT();
    }

    case OpCode::SETIDX:
    {
        Operand rsrc = V->ip->operand1;
        Operand rtbl = V->ip->operand2;
        Operand ridx = V->ip->operand3;

        TValue val;
        TValue tbl = *getregister(V, rtbl.val_register);
        TValue idx = *getregister(V, ridx.val_register);

        // Get table key based on the index type (string or number)
        TableKey key = checkstring(V, idx) ? idx.val_string->hash : static_cast<Hash>(idx.val_number);
#ifdef VIA_DEBUG
        // Assert that the value is a table
        std::string _errfmt = std::format("Attempt to assign index to {}", ENUM_NAME(tbl.type));
        VM_ASSERT(checktable(V, tbl), _errfmt);
#endif
        // Slow-path: the value is stored in a register, load it
        if (VIA_UNLIKELY(rsrc.type == OperandType::GPRegister))
            val = *getregister(V, rsrc.val_register);
        else
            val = toviavalue(V, rsrc);

        // Set the table index
        TTable *T = tbl.val_table;
        settableindex(V, T, key, val);

        VM_NEXT();
    }

    case OpCode::LEN:
    {
        Operand rdst = V->ip->operand1;
        Operand objr = V->ip->operand2;

        TValue val;

        // Fast-path: object is stored in a register
        if (VIA_LIKELY(objr.type == OperandType::GPRegister))
            val = *getregister(V, objr.val_register);
        else
            val = toviavalue(V, objr);

        setregister(V, rdst.val_register, len(V, val));

        VM_NEXT();
    }

    case OpCode::TYPE:
    {
        Operand rdst = V->ip->operand1;
        Operand objr = V->ip->operand2;

        TValue *val = getregister(V, objr.val_register);
        TValue ty = type(V, *val);

        setregister(V, rdst.val_register, ty);

        VM_NEXT();
    }

    case OpCode::TYPEOF:
    {
        Operand rdst = V->ip->operand1;
        Operand objr = V->ip->operand2;

        TValue *val = getregister(V, objr.val_register);
        TValue type = typeof(V, *val);

        setregister(V, rdst.val_register, type);

        VM_NEXT();
    }

    case OpCode::STRCON:
    {
        Operand rdst = V->ip->operand1;
        Operand lhsr = V->ip->operand2;
        Operand rhsr = V->ip->operand3;

        TValue lhs = *getregister(V, lhsr.val_register);
        TValue rhs = *getregister(V, rhsr.val_register);
#ifdef VIA_DEBUG
        VM_ASSERT(checkstring(V, lhs), "Attempt to concatenate non-string value");
        VM_ASSERT(checkstring(V, rhs), "Attempt to concatenate string with non-string value");
#endif
        std::string str = std::string(lhs.val_string->ptr) + rhs.val_string->ptr;
        TString *vstr = newstring(V, str.c_str());

        setregister(V, rdst.val_register, stackvalue(V, vstr));

        VM_NEXT();
    }

    default:
        setexitdata(V, 1, std::format("Unrecognized OpCode (op_id={})", static_cast<uint8_t>(V->ip->op)));
        VM_EXIT();
    }
}

exit:;
} // namespace via

void killthread(RTState *V)
{
    if (V->tstate == ThreadState::RUNNING)
        // TODO: Wait for the VM to exit
        V->abrt = true;

    // Mark as dead thread
    V->tstate = ThreadState::DEAD;
    // Decrement the thread_id to make room for more threads (I know you can technically make 4 billion threads ok?)
    __thread_id__--;
}

void pausethread(RTState *V)
{
    V->tstate = ThreadState::PAUSED;
    // Save the VM state to restore it when paused
    savestate(V);
}

} // namespace via