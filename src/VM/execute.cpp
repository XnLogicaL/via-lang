/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "execute.h"
#include "api.h"
#include "chunk.h"
#include "state.h"
#include "types.h"

// Define the hot path threshold for the instruction dispatch loop
// How many times a chunk needs to be executed before being flagged as "hot"
#ifndef VIA_HOTPATH_THRESHOLD
    #define VIA_HOTPATH_THRESHOLD 64
#endif

// Check for debug mode and warn the user about it's possible effects on behavior
#ifdef VIA_DEBUG
    #pragma message(Compiling via in debug mode; some optimizations may be disabled)
    #ifdef VIA_ALLOW_OPTIMIZATIONS_IN_DEBUG_MODE
        #pragma message(Enabling optimizations in debug mode may cause instability issues)
    #endif
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
            setexitdata(V, 1, std::format("VM_ASSERT(): {}\n in file {}, line {}", (message), __FILE__, __LINE__)); \
            VM_EXIT(); \
        } \
    }

namespace via
{

// Get compilation stuff in the namespace
using namespace Compilation;

// Internal function that optimizes a sequence of empty instructions (such as NOP or END)
// By replacing the first instruction with a jmp instruction that jumps over the sequence
VIA_FORCEINLINE void optimize_empty_instruction_sequence(RTState *VIA_RESTRICT V)
{
    // Check for an empty instruction at the current IP
    if (VIA_UNLIKELY(V->ip->op == OpCode::NOP))
    {
        JmpOffset skip_count = 1;

        // Find the end of the sequence of empty instructions
        while (V->ip + skip_count < V->ibp && V->ip->op == OpCode::NOP)
            skip_count++;

        // If there are multiple empty instructions, replace the first one with a JMP
        if (skip_count > 1)
        {
            V->ip->op = OpCode::JMP;
            V->ip->operand1 = Compilation::cnewoperand(static_cast<double>(skip_count));
        }
    }
}

// Starts VM execution cycle by altering it's state and "iterating" over the instruction pipeline.
void execute(RTState *VIA_RESTRICT V)
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
        // This is necessary because the old instruction pointer may be dangling from now on
        VM_NEXT();
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
    case OpCode::NOP:
    {
        // Optimizations that modify the program are not allowed in debug mode
        // Unless a very certain flag is provided
#if !defined(VIA_DEBUG) || defined(VIA_DEBUG) && defined(VIA_ALLOW_OPTIMIZATIONS_IN_DEBUG_MODE)
        // Attempt to optimize empty instruction sequence
        optimize_empty_instruction_sequence(V);
#endif
        VM_NEXT();
    }

    case OpCode::MOV:
    {
        Operand rdst = V->ip->operand1;
        Operand rsrc = V->ip->operand2;
        TValue *src_val = rgetregister(V->ralloc, rsrc.val_register);

        rsetregister(V->ralloc, rdst.val_register, *src_val);
        VM_NEXT();
    }

    case OpCode::LOADNIL:
    {
        Operand dst = V->ip->operand1;
        TValue tnil = stackvalue(V);

        rsetregister(V->ralloc, dst.val_register, tnil);
        VM_NEXT();
    }

    case OpCode::LOADTABLE:
    {
        Operand dst = V->ip->operand1;
        TValue ttable = stackvalue(V, newtable(V));

        rsetregister(V->ralloc, dst.val_register, ttable);
        VM_NEXT();
    }

    case OpCode::LOADBOOL:
    {
        Operand dst = V->ip->operand1;
        Operand val = V->ip->operand2;
        TValue tbool = stackvalue(V, val.val_boolean);

        rsetregister(V->ralloc, dst.val_register, tbool);
        VM_NEXT();
    }

    case OpCode::LOADNUMBER:
    {
        Operand dst = V->ip->operand1;
        Operand val = V->ip->operand2;
        TValue tnumber = stackvalue(V, val.val_number);

        rsetregister(V->ralloc, dst.val_register, tnumber);
        VM_NEXT();
    }

    case OpCode::LOADSTRING:
    {
        Operand dst = V->ip->operand1;
        Operand val = V->ip->operand2;
        TValue tstring = stackvalue(V, val.val_string);

        rsetregister(V->ralloc, dst.val_register, tstring);
        VM_NEXT();
    }

    case OpCode::LOADFUNCTION:
    {
        Operand dst = V->ip->operand1;
        TFunction *func = newfunc(V, "<anonymous>", V->ip, {}, false, false);

        while (V->ip < V->ibp)
        {
            if (V->ip->op == OpCode::RET)
            {
                V->ip++;
                break;
            }
            // Copy the instruction and insert into the function object
            func->bytecode.push_back(*(V->ip++));
        }

        rsetregister(V->ralloc, dst.val_register, stackvalue(V, func));
        // Dispatch instead of invoking VM_NEXT
        VM_DISPATCH();
    }

    case OpCode::PUSH:
    {
        Operand src = V->ip->operand1;
        TValue *val = rgetregister(V->ralloc, src.val_register);

        tspush(V->stack, val);
        VM_NEXT();
    }

    case OpCode::POP:
    {
        Operand dst = V->ip->operand1;
        TValue *val = popval(V);

        rsetregister(V->ralloc, dst.val_register, *val);
        VM_NEXT();
    }

    case OpCode::GETSTACK:
    {
        Operand dst = V->ip->operand1;
        Operand off = V->ip->operand2;

        StkPos stack_offset = static_cast<StkPos>(off.val_number);
        StkVal val_ptr = *(V->stack->sbp + stack_offset);
        TValue *val = reinterpret_cast<TValue *>(val_ptr);

        rsetregister(V->ralloc, dst.val_register, *val);
        VM_NEXT();
    }

    case OpCode::SETSTACK:
    {
        Operand src = V->ip->operand1;
        Operand off = V->ip->operand2;

        StkPos stack_offset = static_cast<StkPos>(off.val_number);
        TValue *val = rgetregister(V->ralloc, src.val_register);
        StkVal val_ptr = reinterpret_cast<StkVal>(val);

        *(V->stack->sbp + stack_offset) = val_ptr;
        VM_NEXT();
    }

    case OpCode::GETARGUMENT:
    {
        Operand dst = V->ip->operand1;
        Operand off = V->ip->operand2;

        LocalId offv = static_cast<LocalId>(off.val_number);
        TValue *val = getargument(V, offv);

        rsetregister(V->ralloc, dst.val_register, *val);
        VM_NEXT();
    }

    case OpCode::ADDRR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        TValue result = arith(V, *lhsv, *rhsv, OpCode::ADDRR);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::ADDRN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue rhsv = stackvalue(V, rhs.val_number);
        TValue result = arith(V, *lhsv, rhsv, OpCode::ADDRN);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::ADDNR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsv = stackvalue(V, lhs.val_number);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        TValue result = arith(V, lhsv, *rhsv, OpCode::ADDNR);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::ADDNN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsv = stackvalue(V, lhs.val_number);
        TValue rhsv = stackvalue(V, rhs.val_number);
        TValue result = arith(V, lhsv, rhsv, OpCode::ADDNN);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::ADDIR:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        iarith(V, lhsv, *rhsv, OpCode::ADDIR);
        VM_NEXT();
    }
    case OpCode::ADDIN:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue rhsv = stackvalue(V, rhs.val_number);

        iarith(V, lhsv, rhsv, OpCode::ADDIN);
        VM_NEXT();
    }

    case OpCode::SUBRR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        TValue result = arith(V, *lhsv, *rhsv, OpCode::SUBRR);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::SUBRN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue rhsv = stackvalue(V, rhs.val_number);
        TValue result = arith(V, *lhsv, rhsv, OpCode::SUBRN);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::SUBNR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsv = stackvalue(V, lhs.val_number);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        TValue result = arith(V, lhsv, *rhsv, OpCode::SUBNR);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::SUBNN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsv = stackvalue(V, lhs.val_number);
        TValue rhsv = stackvalue(V, rhs.val_number);
        TValue result = arith(V, lhsv, rhsv, OpCode::SUBNN);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::SUBIR:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        iarith(V, lhsv, *rhsv, OpCode::SUBIR);
        VM_NEXT();
    }
    case OpCode::SUBIN:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        iarith(V, lhsv, *rhsv, OpCode::SUBIN);
        VM_NEXT();
    }

    case OpCode::MULRR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        TValue result = arith(V, *lhsv, *rhsv, OpCode::MULRR);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::MULRN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue rhsv = stackvalue(V, rhs.val_number);
        TValue result = arith(V, *lhsv, rhsv, OpCode::MULRN);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::MULNR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsv = stackvalue(V, lhs.val_number);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        TValue result = arith(V, lhsv, *rhsv, OpCode::MULNR);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::MULNN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsv = stackvalue(V, lhs.val_number);
        TValue rhsv = stackvalue(V, rhs.val_number);
        TValue result = arith(V, lhsv, rhsv, OpCode::MULNN);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::MULIR:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        iarith(V, lhsv, *rhsv, OpCode::MULIR);
        VM_NEXT();
    }
    case OpCode::MULIN:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        iarith(V, lhsv, *rhsv, OpCode::MULIN);
        VM_NEXT();
    }

    case OpCode::DIVRR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        TValue result = arith(V, *lhsv, *rhsv, OpCode::DIVRR);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::DIVRN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue rhsv = stackvalue(V, rhs.val_number);
        TValue result = arith(V, *lhsv, rhsv, OpCode::DIVRN);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::DIVNR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsv = stackvalue(V, lhs.val_number);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        TValue result = arith(V, lhsv, *rhsv, OpCode::DIVNR);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::DIVNN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsv = stackvalue(V, lhs.val_number);
        TValue rhsv = stackvalue(V, rhs.val_number);
        TValue result = arith(V, lhsv, rhsv, OpCode::DIVNN);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::DIVIR:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        iarith(V, lhsv, *rhsv, OpCode::DIVIR);
        VM_NEXT();
    }
    case OpCode::DIVIN:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        iarith(V, lhsv, *rhsv, OpCode::DIVIN);
        VM_NEXT();
    }

    case OpCode::POWRR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        TValue result = arith(V, *lhsv, *rhsv, OpCode::POWRR);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::POWRN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue rhsv = stackvalue(V, rhs.val_number);
        TValue result = arith(V, *lhsv, rhsv, OpCode::POWRN);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::POWNR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsv = stackvalue(V, lhs.val_number);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        TValue result = arith(V, lhsv, *rhsv, OpCode::POWNR);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::POWNN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsv = stackvalue(V, lhs.val_number);
        TValue rhsv = stackvalue(V, rhs.val_number);
        TValue result = arith(V, lhsv, rhsv, OpCode::POWNN);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::POWIR:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        iarith(V, lhsv, *rhsv, OpCode::POWIR);
        VM_NEXT();
    }
    case OpCode::POWIN:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        iarith(V, lhsv, *rhsv, OpCode::POWIN);
        VM_NEXT();
    }

    case OpCode::MODRR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        TValue result = arith(V, *lhsv, *rhsv, OpCode::MODRR);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::MODRN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue rhsv = stackvalue(V, rhs.val_number);
        TValue result = arith(V, *lhsv, rhsv, OpCode::MODRN);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::MODNR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsv = stackvalue(V, lhs.val_number);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        TValue result = arith(V, lhsv, *rhsv, OpCode::MODNR);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::MODNN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsv = stackvalue(V, lhs.val_number);
        TValue rhsv = stackvalue(V, rhs.val_number);
        TValue result = arith(V, lhsv, rhsv, OpCode::MODNN);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::MODIR:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        iarith(V, lhsv, *rhsv, OpCode::MODIR);
        VM_NEXT();
    }
    case OpCode::MODIN:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        iarith(V, lhsv, *rhsv, OpCode::MODIN);
        VM_NEXT();
    }

    case OpCode::NEGR:
    {
        Operand dst = V->ip->operand1;
        Operand src = V->ip->operand2;

        TValue *srcv = rgetregister(V->ralloc, src.val_register);
        TValue result = tobool(V, *srcv);
        result.val_boolean = !result.val_boolean;

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::NEGI:
    {
        Operand dst = V->ip->operand1;

        TValue *dstv = rgetregister(V->ralloc, dst.val_register);
        TValue result = tobool(V, *dstv);
        dstv->val_boolean = !result.val_boolean;

        VM_NEXT();
    }

    case OpCode::BANDRR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhsv->val_number);

        TNumber resultb = lhs_val & rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BANDRN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhs.val_number);

        TNumber resultb = lhs_val & rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BANDNR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        int64_t lhs_val = static_cast<int64_t>(lhs.val_boolean);
        int64_t rhs_val = static_cast<int64_t>(rhsv->val_number);

        TNumber resultb = lhs_val & rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BANDNN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        int64_t lhs_val = static_cast<int64_t>(lhs.val_number);
        int64_t rhs_val = static_cast<int64_t>(rhs.val_number);

        TNumber resultb = lhs_val & rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BANDIR:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhsv->val_number);

        lhsv->val_number = static_cast<TNumber>(lhs_val & rhs_val);

        VM_NEXT();
    }
    case OpCode::BANDIN:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);

        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhs.val_number);

        lhsv->val_number = static_cast<TNumber>(lhs_val & rhs_val);

        VM_NEXT();
    }

    case OpCode::BORRR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhsv->val_number);

        TNumber resultb = lhs_val | rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BORRN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhs.val_number);

        TNumber resultb = lhs_val | rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BORNR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        int64_t lhs_val = static_cast<int64_t>(lhs.val_boolean);
        int64_t rhs_val = static_cast<int64_t>(rhsv->val_number);

        TNumber resultb = lhs_val | rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BORNN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        int64_t lhs_val = static_cast<int64_t>(lhs.val_number);
        int64_t rhs_val = static_cast<int64_t>(rhs.val_number);

        TNumber resultb = lhs_val | rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BORIR:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhsv->val_number);

        lhsv->val_number = static_cast<TNumber>(lhs_val | rhs_val);

        VM_NEXT();
    }
    case OpCode::BORIN:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);

        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhs.val_number);

        lhsv->val_number = static_cast<TNumber>(lhs_val | rhs_val);

        VM_NEXT();
    }

    case OpCode::BXORRR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhsv->val_number);

        TNumber resultb = lhs_val ^ rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BXORRN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhs.val_number);

        TNumber resultb = lhs_val ^ rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BXORNR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        int64_t lhs_val = static_cast<int64_t>(lhs.val_boolean);
        int64_t rhs_val = static_cast<int64_t>(rhsv->val_number);

        TNumber resultb = lhs_val ^ rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BXORNN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        int64_t lhs_val = static_cast<int64_t>(lhs.val_number);
        int64_t rhs_val = static_cast<int64_t>(rhs.val_number);

        TNumber resultb = lhs_val ^ rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BXORIR:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhsv->val_number);

        lhsv->val_number = static_cast<TNumber>(lhs_val ^ rhs_val);

        VM_NEXT();
    }
    case OpCode::BXORIN:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);

        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhs.val_number);

        lhsv->val_number = static_cast<TNumber>(lhs_val ^ rhs_val);

        VM_NEXT();
    }

    case OpCode::BNOTR:
    {
        Operand dst = V->ip->operand1;
        Operand src = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, dst.val_register);
        int64_t rhsv = static_cast<int64_t>(src.val_number);
        lhsv->val_number = static_cast<TNumber>(~rhsv);

        VM_NEXT();
    }
    case OpCode::BNOTI:
    {
        Operand dst = V->ip->operand1;

        TValue *lhsv = rgetregister(V->ralloc, dst.val_register);
        uint64_t result = ~static_cast<uint64_t>(lhsv->val_number);
        lhsv->val_number = static_cast<TNumber>(result);

        VM_NEXT();
    }

    case OpCode::BSHLRR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhsv->val_number);

        TNumber resultb = lhs_val << rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BSHLRN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhs.val_number);

        TNumber resultb = lhs_val << rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BSHLNR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        int64_t lhs_val = static_cast<int64_t>(lhs.val_boolean);
        int64_t rhs_val = static_cast<int64_t>(rhsv->val_number);

        TNumber resultb = lhs_val << rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BSHLNN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        int64_t lhs_val = static_cast<int64_t>(lhs.val_number);
        int64_t rhs_val = static_cast<int64_t>(rhs.val_number);

        TNumber resultb = lhs_val << rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BSHLIR:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhsv->val_number);

        lhsv->val_number = static_cast<TNumber>(lhs_val << rhs_val);

        VM_NEXT();
    }
    case OpCode::BSHLIN:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);

        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhs.val_number);

        lhsv->val_number = static_cast<TNumber>(lhs_val << rhs_val);

        VM_NEXT();
    }

    case OpCode::BSHRRR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhsv->val_number);

        TNumber resultb = lhs_val >> rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BSHRRN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhs.val_number);

        TNumber resultb = lhs_val >> rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BSHRNR:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);
        int64_t lhs_val = static_cast<int64_t>(lhs.val_boolean);
        int64_t rhs_val = static_cast<int64_t>(rhsv->val_number);

        TNumber resultb = lhs_val >> rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BSHRNN:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        int64_t lhs_val = static_cast<int64_t>(lhs.val_number);
        int64_t rhs_val = static_cast<int64_t>(rhs.val_number);

        TNumber resultb = lhs_val >> rhs_val;
        TValue result = stackvalue(V, resultb);

        rsetregister(V->ralloc, dst.val_register, result);
        VM_NEXT();
    }
    case OpCode::BSHRIR:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsv = rgetregister(V->ralloc, rhs.val_register);

        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhsv->val_number);

        lhsv->val_number = static_cast<TNumber>(lhs_val >> rhs_val);

        VM_NEXT();
    }
    case OpCode::BSHRIN:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhsv = rgetregister(V->ralloc, lhs.val_register);

        int64_t lhs_val = static_cast<int64_t>(lhsv->val_number);
        int64_t rhs_val = static_cast<int64_t>(rhs.val_number);

        lhsv->val_number = static_cast<TNumber>(lhs_val >> rhs_val);

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

        if (VIA_UNLIKELY(lhs.type == OperandType::Register))
        {
            lhsn = *rgetregister(V->ralloc, lhs.val_register);
            lhs_reg = true;
        }
        else
            lhsn = toviavalue(V, lhs);

        if (VIA_UNLIKELY(rhs.type == OperandType::Register))
        {
            rhsn = *rgetregister(V->ralloc, rhs.val_register);
            rhs_reg = true;
        }
        else
            rhsn = toviavalue(V, rhs);

        if (lhs_reg && rhs_reg)
            rsetregister(V->ralloc, dst.val_register, stackvalue(V, !cmpregister(V, lhs.val_register, rhs.val_register)));
        else if (lhs_reg)
            rsetregister(V->ralloc, dst.val_register, stackvalue(V, !compare(V, *rgetregister(V->ralloc, lhs.val_register), rhsn)));
        else if (rhs_reg)
            rsetregister(V->ralloc, dst.val_register, stackvalue(V, !compare(V, *rgetregister(V->ralloc, rhs.val_register), lhsn)));
        else
            rsetregister(V->ralloc, dst.val_register, stackvalue(V, !compare(V, lhsn, rhsn)));

        VM_NEXT();
    }
    case OpCode::LT:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsn = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsn = rgetregister(V->ralloc, rhs.val_register);

        if (VIA_LIKELY(checknumber(V, *lhsn)))
        {
            rsetregister(V->ralloc, dst.val_register, stackvalue(V, lhsn->val_number < rhsn->val_number));
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(checktable(V, *lhsn)))
        {
            TValue *mm = getmetamethod(V, *lhsn, OpCode::LT);

            tspush(V->stack, lhsn);
            tspush(V->stack, lhsn);
            call(V, *mm, 2);

            StkVal ptr = tspop(V->stack);
            TValue *val = reinterpret_cast<TValue *>(ptr);

            rsetregister(V->ralloc, dst.val_register, *val);
            VM_NEXT();
        }

        VM_NEXT();
    }
    case OpCode::GT:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsn = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsn = rgetregister(V->ralloc, rhs.val_register);

        if (VIA_LIKELY(checknumber(V, *lhsn)))
        {
            rsetregister(V->ralloc, dst.val_register, stackvalue(V, lhsn->val_number > rhsn->val_number));
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(checktable(V, *lhsn)))
        {
            TValue *mm = getmetamethod(V, *lhsn, OpCode::LT);

            tspush(V->stack, lhsn);
            tspush(V->stack, rhsn);
            call(V, *mm, 2);

            StkVal ptr = tspop(V->stack);
            TValue *val = reinterpret_cast<TValue *>(ptr);

            rsetregister(V->ralloc, dst.val_register, *val);
            VM_NEXT();
        }

        VM_NEXT();
    }
    case OpCode::LE:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsn = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsn = rgetregister(V->ralloc, rhs.val_register);

        if (VIA_LIKELY(checknumber(V, *lhsn)))
        {
            rsetregister(V->ralloc, dst.val_register, stackvalue(V, lhsn->val_number <= rhsn->val_number));
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(checktable(V, *lhsn)))
        {
            TValue *mm = getmetamethod(V, *lhsn, OpCode::LT);

            tspush(V->stack, lhsn);
            tspush(V->stack, rhsn);
            call(V, *mm, 2);

            StkVal ptr = tspop(V->stack);
            TValue *val = reinterpret_cast<TValue *>(ptr);

            rsetregister(V->ralloc, dst.val_register, *val);
            VM_NEXT();
        }

        VM_NEXT();
    }
    case OpCode::GE:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsn = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsn = rgetregister(V->ralloc, rhs.val_register);

        if (VIA_LIKELY(checknumber(V, *lhsn)))
        {
            rsetregister(V->ralloc, dst.val_register, stackvalue(V, lhsn->val_number >= rhsn->val_number));
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(checktable(V, *lhsn)))
        {
            TValue *mm = getmetamethod(V, *lhsn, OpCode::LT);

            tspush(V->stack, lhsn);
            tspush(V->stack, rhsn);
            call(V, *mm, 2);

            StkVal ptr = tspop(V->stack);
            TValue *val = reinterpret_cast<TValue *>(ptr);

            rsetregister(V->ralloc, dst.val_register, *val);
            VM_NEXT();
        }

        VM_NEXT();
    }

    case OpCode::EXIT:
    {
        Operand rcode = V->ip->operand1;

        TValue *code = rgetregister(V->ralloc, rcode.val_register);
        TNumber ecode = tonumber(V, *code).val_number;

        setexitdata(V, static_cast<ExitCode>(ecode), "VM exited by user");
        VM_EXIT();
    }

    case OpCode::JMP:
    {
        Operand offset = V->ip->operand1;
        V->ip += static_cast<JmpOffset>(offset.val_number);
        VM_NEXT();
    }

    case OpCode::JMPNZ:
    case OpCode::JMPZ:
    {
        Operand condr = V->ip->operand1;
        Operand offset = V->ip->operand2;

        TValue cond = *rgetregister(V->ralloc, condr.val_register);
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

        TValue *lhs = rgetregister(V->ralloc, condlr.val_register);
        TValue *rhs = rgetregister(V->ralloc, condrr.val_register);

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

        TValue *lhs = rgetregister(V->ralloc, condlr.val_register);
        TValue *rhs = rgetregister(V->ralloc, condrr.val_register);

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

        TValue *lhs = rgetregister(V->ralloc, condlr.val_register);
        TValue *rhs = rgetregister(V->ralloc, condrr.val_register);

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

        TValue *lhs = rgetregister(V->ralloc, condlr.val_register);
        TValue *rhs = rgetregister(V->ralloc, condrr.val_register);

        bool cond = lhs->val_number >= rhs->val_number;
        if (cond)
            V->ip += static_cast<JmpOffset>(offset.val_number);

        VM_NEXT();
    }

    case OpCode::JMPLBL:
    {
        Operand label = V->ip->operand1;
        auto it = V->labels->find(std::string_view(label.val_identifier));
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
        TValue *val = rgetregister(V->ralloc, valr.val_register);
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
        bool cond = rgetregister(V->ralloc, lhsr.val_register)->val_number < rgetregister(V->ralloc, rhsr.val_register)->val_number;
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
        bool cond = rgetregister(V->ralloc, lhsr.val_register)->val_number > rgetregister(V->ralloc, rhsr.val_register)->val_number;
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
        bool cond = rgetregister(V->ralloc, lhsr.val_register)->val_number <= rgetregister(V->ralloc, rhsr.val_register)->val_number;
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
        bool cond = rgetregister(V->ralloc, lhsr.val_register)->val_number >= rgetregister(V->ralloc, rhsr.val_register)->val_number;
        // Jump if the condition is met
        if (cond)
            V->ip = it->second + 1;

        VM_NEXT();
    }

    case OpCode::CALL:
    {
        Operand rfn = V->ip->operand1;
        Operand argco = V->ip->operand2;
        CallArgc argc = static_cast<CallArgc>(argco.val_number);
        // Call function
        call(V, *rgetregister(V->ralloc, rfn.val_register), argc);
        VM_NEXT();
    }
    case OpCode::EXTERNCALL:
    {
        Operand rfn = V->ip->operand1;
        Operand argco = V->ip->operand2;
        CallArgc argc = static_cast<CallArgc>(argco.val_number);
        TValue *cfunc = rgetregister(V->ralloc, rfn.val_register);

        // Call function
        externcall(V, cfunc->val_cfunction, argc);
        VM_NEXT();
    }
    case OpCode::NATIVECALL:
    {
        Operand rfn = V->ip->operand1;
        Operand argco = V->ip->operand2;
        CallArgc argc = static_cast<CallArgc>(argco.val_number);
        TValue *func = rgetregister(V->ralloc, rfn.val_register);

        // Call function
        nativecall(V, func->val_function, argc);
        VM_NEXT();
    }
    case OpCode::METHODCALL:
    {
        Operand robj = V->ip->operand1;
        Operand rfn = V->ip->operand2;
        Operand argco = V->ip->operand3;

        CallArgc argc = static_cast<CallArgc>(argco.val_number);
        TValue *func = rgetregister(V->ralloc, rfn.val_register);
        TValue *obj = rgetregister(V->ralloc, robj.val_register);

        pushval(V, obj);
        // Call function, with [argc + 1] to account for the self argument
        nativecall(V, func->val_function, argc + 1);
        VM_NEXT();
    }

    case OpCode::RET:
    {
        Operand retcv = V->ip->operand1;
        CallArgc retc = static_cast<CallArgc>(retcv.val_number);

        nativeret(V, retc);
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
            V->ip++;
            if (V->ip->op == OpCode::RET)
                break;
        }

        VM_DISPATCH();
    }

    case OpCode::GETTABLE:
    {
        Operand rdst = V->ip->operand1;
        Operand rtbl = V->ip->operand2;
        Operand ridx = V->ip->operand3;

        TValue tbl = *rgetregister(V->ralloc, rtbl.val_register);
        TValue idx = *rgetregister(V->ralloc, ridx.val_register);

        // Get table key based on the index type (string or number)
        TableKey key = checkstring(V, idx) ? idx.val_string->hash : idx.val_number;
        // Load the table index
        loadtableindex(V, tbl.val_table, key, rdst.val_register);
        VM_NEXT();
    }

    case OpCode::SETTABLE:
    {
        Operand rsrc = V->ip->operand1;
        Operand rtbl = V->ip->operand2;
        Operand ridx = V->ip->operand3;

        TValue val;
        TValue tbl = *rgetregister(V->ralloc, rtbl.val_register);
        TValue idx = *rgetregister(V->ralloc, ridx.val_register);

        // Get table key based on the index type (string or number)
        TableKey key = checkstring(V, idx) ? idx.val_string->hash : static_cast<Hash>(idx.val_number);
        // Slow-path: the value is stored in a register, load it
        if (VIA_UNLIKELY(rsrc.type == OperandType::Register))
            val = *rgetregister(V->ralloc, rsrc.val_register);
        else
            val = toviavalue(V, rsrc);

        // Set the table index
        TTable *T = tbl.val_table;
        settableindex(V, T, key, val);
        VM_NEXT();
    }

    case OpCode::LENSTRING:
    {
        Operand rdst = V->ip->operand1;
        Operand objr = V->ip->operand2;

        TValue *val = rgetregister(V->ralloc, objr.val_register);
        TNumber len = static_cast<TNumber>(val->val_string->len);

        rsetregister(V->ralloc, rdst.val_register, stackvalue(V, len));
        VM_NEXT();
    }

    case OpCode::TYPE:
    {
        Operand rdst = V->ip->operand1;
        Operand objr = V->ip->operand2;

        TValue *val = rgetregister(V->ralloc, objr.val_register);
        TValue ty = type(V, *val);

        rsetregister(V->ralloc, rdst.val_register, ty);

        VM_NEXT();
    }

    case OpCode::TYPEOF:
    {
        Operand rdst = V->ip->operand1;
        Operand objr = V->ip->operand2;

        TValue *val = rgetregister(V->ralloc, objr.val_register);
        TValue type = typeofv(V, *val);

        rsetregister(V->ralloc, rdst.val_register, type);

        VM_NEXT();
    }

    case OpCode::CONCATRR:
    {
        Operand rdst = V->ip->operand1;
        Operand lhsr = V->ip->operand2;
        Operand rhsr = V->ip->operand3;

        TValue lhs = *rgetregister(V->ralloc, lhsr.val_register);
        TValue rhs = *rgetregister(V->ralloc, rhsr.val_register);
        std::string str = std::string(lhs.val_string->ptr) + rhs.val_string->ptr;
        TString *vstr = newstring(V, str.c_str());

        rsetregister(V->ralloc, rdst.val_register, stackvalue(V, vstr));
        VM_NEXT();
    }

    default:
        setexitdata(V, 1, std::format("Unrecognized OpCode (op_id={})", static_cast<uint8_t>(V->ip->op)));
        VM_EXIT();
    }
}

exit:;
}

// Permanently kills the thread. Does not clean up the state object.
void killthread(RTState *VIA_RESTRICT V)
{
    if (V->tstate == ThreadState::RUNNING)
        // TODO: Wait for the VM to exit
        V->abrt = true;

    // Mark as dead thread
    V->tstate = ThreadState::DEAD;
    // Decrement the thread_id to make room for more threads (I know you can technically make 4 billion threads ok?)
    V->G->threads--;
}

// Temporarily pauses the thread. Saves the state.
void pausethread(RTState *VIA_RESTRICT V)
{
    V->tstate = ThreadState::PAUSED;
    // Save the VM state to restore it when paused
    savestate(V);
}

} // namespace via
