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

// Starts VM execution cycle by altering it's state and "iterating" over the instruction pipeline.
void execute(RTState *VIA_RESTRICT V)
{
    VIA_ASSERT(V->tstate == ThreadState::PAUSED, "execute() must be called on paused thread");
    V->tstate = ThreadState::RUNNING;

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
        VM_NEXT();

    case OpCode::ADD:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhs_val = rgetregister(V->ralloc, rhs.val_register);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val->val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::ADD);
            pushval(V, *lhs_val); // Push self
            pushval(V, *rhs_val); // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::ADDK:
    {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        size_t const_idx = idx.val_number;
        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue rhs_val = V->G->ktable->at(const_idx);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::ADD);
            pushval(V, *lhs_val); // Push self
            pushval(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::ADDI:
    {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue rhs_val = toviavalue(V, imm);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::ADD);
            pushval(V, *lhs_val); // Push self
            pushval(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::SUB:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhs_val = rgetregister(V->ralloc, rhs.val_register);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val->val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::SUB);
            pushval(V, *lhs_val); // Push self
            pushval(V, *rhs_val); // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::SUBK:
    {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        size_t const_idx = idx.val_number;
        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue rhs_val = V->G->ktable->at(const_idx);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::SUB);
            pushval(V, *lhs_val); // Push self
            pushval(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::SUBI:
    {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue rhs_val = toviavalue(V, imm);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::SUB);
            pushval(V, *lhs_val); // Push self
            pushval(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::MUL:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhs_val = rgetregister(V->ralloc, rhs.val_register);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val->val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::MUL);
            pushval(V, *lhs_val); // Push self
            pushval(V, *rhs_val); // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::MULK:
    {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        size_t const_idx = idx.val_number;
        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue rhs_val = V->G->ktable->at(const_idx);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::MUL);
            pushval(V, *lhs_val); // Push self
            pushval(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::MULI:
    {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue rhs_val = toviavalue(V, imm);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::MUL);
            pushval(V, *lhs_val); // Push self
            pushval(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::DIV:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhs_val = rgetregister(V->ralloc, rhs.val_register);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val->val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::DIV);
            pushval(V, *lhs_val); // Push self
            pushval(V, *rhs_val); // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::DIVK:
    {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        size_t const_idx = idx.val_number;
        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue rhs_val = V->G->ktable->at(const_idx);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::DIV);
            pushval(V, *lhs_val); // Push self
            pushval(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::DIVI:
    {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue rhs_val = toviavalue(V, imm);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::DIV);
            pushval(V, *lhs_val); // Push self
            pushval(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::POW:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhs_val = rgetregister(V->ralloc, rhs.val_register);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val->val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::POW);
            pushval(V, *lhs_val); // Push self
            pushval(V, *rhs_val); // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::POWK:
    {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        size_t const_idx = idx.val_number;
        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue rhs_val = V->G->ktable->at(const_idx);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::POW);
            pushval(V, *lhs_val); // Push self
            pushval(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::POWI:
    {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue rhs_val = toviavalue(V, imm);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::POW);
            pushval(V, *lhs_val); // Push self
            pushval(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::MOD:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhs_val = rgetregister(V->ralloc, rhs.val_register);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val->val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::MOD);
            pushval(V, *lhs_val); // Push self
            pushval(V, *rhs_val); // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::MODK:
    {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        size_t const_idx = idx.val_number;
        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue rhs_val = V->G->ktable->at(const_idx);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::MOD);
            pushval(V, *lhs_val); // Push self
            pushval(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::MODI:
    {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = rgetregister(V->ralloc, lhs.val_register);
        TValue rhs_val = toviavalue(V, imm);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            TValue metamethod = getmetamethod(V, *lhs_val, OpCode::MOD);
            pushval(V, *lhs_val); // Push self
            pushval(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::MOVE:
    {
        Operand rdst = V->ip->operand1;
        Operand rsrc = V->ip->operand2;
        TValue *src_val = rgetregister(V->ralloc, rsrc.val_register);

        rsetregister(V->ralloc, rdst.val_register, *src_val);
        VM_NEXT();
    }

    case OpCode::LOADK:
    {
        Operand dst = V->ip->operand1;
        Operand idx = V->ip->operand2;
        kTable::size_type kid = idx.val_number;

        // Check if the kId is valid
        VM_ASSERT(kid > V->G->ktable->size(), "LOADK: Invalid constant index (out of range)");

        TValue kval = V->G->ktable->at(kid);

        rsetregister(V->ralloc, dst.val_register, kval);
        VM_NEXT();
    }

    case OpCode::LOADNIL:
    {
        Operand dst = V->ip->operand1;
        TValue tnil = TValue();

        rsetregister(V->ralloc, dst.val_register, tnil);
        VM_NEXT();
    }

    case OpCode::LOADTABLE:
    {
        Operand dst = V->ip->operand1;
        TValue ttable = TValue(new TTable());

        rsetregister(V->ralloc, dst.val_register, ttable);
        VM_NEXT();
    }

    case OpCode::LOADBOOL:
    {
        Operand dst = V->ip->operand1;
        Operand val = V->ip->operand2;
        TValue tbool = TValue(val.val_boolean);

        rsetregister(V->ralloc, dst.val_register, tbool);
        VM_NEXT();
    }

    case OpCode::LOADNUMBER:
    {
        Operand dst = V->ip->operand1;
        Operand val = V->ip->operand2;
        TValue tnumber = TValue(val.val_number);

        rsetregister(V->ralloc, dst.val_register, tnumber);
        VM_NEXT();
    }

    case OpCode::LOADSTRING:
    {
        Operand dst = V->ip->operand1;
        Operand val = V->ip->operand2;
        TValue tstring = TValue(val.val_string);

        rsetregister(V->ralloc, dst.val_register, tstring);
        VM_NEXT();
    }

    case OpCode::LOADFUNCTION:
    {
        Operand dst = V->ip->operand1;
        TFunction *func = new TFunction(V, "<anonymous>", V->ip, V->frame, {}, false, false);

        while (V->ip < V->ibp)
        {
            if (V->ip->op == OpCode::RETURN)
            {
                V->ip++;
                break;
            }
            // Copy the instruction and insert into the function object
            func->bytecode.push_back(*(V->ip++));
        }

        rsetregister(V->ralloc, dst.val_register, TValue(func));
        // Dispatch instead of invoking VM_NEXT
        VM_DISPATCH();
    }

    case OpCode::PUSHSTACK:
    {
        Operand src = V->ip->operand1;
        TValue *val = rgetregister(V->ralloc, src.val_register);

        tspush(V->stack, *val);
        VM_NEXT();
    }

    case OpCode::POPSTACK:
    {
        Operand dst = V->ip->operand1;
        TValue val = popval(V);

        rsetregister(V->ralloc, dst.val_register, val);
        VM_NEXT();
    }

    case OpCode::GETSTACK:
    {
        Operand dst = V->ip->operand1;
        Operand off = V->ip->operand2;

        StkPos stack_offset = static_cast<StkPos>(off.val_number);
        StkVal val = *(V->stack->sbp + stack_offset);

        rsetregister(V->ralloc, dst.val_register, val);
        VM_NEXT();
    }

    case OpCode::SETSTACK:
    {
        Operand src = V->ip->operand1;
        Operand off = V->ip->operand2;

        StkPos stack_offset = static_cast<StkPos>(off.val_number);
        TValue *val = rgetregister(V->ralloc, src.val_register);

        *(V->stack->sbp + stack_offset) = *val;
        VM_NEXT();
    }

    case OpCode::GETARGUMENT:
    {
        Operand dst = V->ip->operand1;
        Operand off = V->ip->operand2;

        LocalId offv = static_cast<LocalId>(off.val_number);
        TValue val = getargument(V, offv);

        rsetregister(V->ralloc, dst.val_register, val);
        VM_NEXT();
    }

    case OpCode::NOTEQUAL:
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
            rsetregister(V->ralloc, dst.val_register, TValue(!compareregisters(V, lhs.val_register, rhs.val_register)));
        else if (lhs_reg)
            rsetregister(V->ralloc, dst.val_register, TValue(!compare(V, *rgetregister(V->ralloc, lhs.val_register), rhsn)));
        else if (rhs_reg)
            rsetregister(V->ralloc, dst.val_register, TValue(!compare(V, *rgetregister(V->ralloc, rhs.val_register), lhsn)));
        else
            rsetregister(V->ralloc, dst.val_register, TValue(!compare(V, lhsn, rhsn)));

        VM_NEXT();
    }
    case OpCode::LESS:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsn = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsn = rgetregister(V->ralloc, rhs.val_register);

        if (VIA_LIKELY(checknumber(V, *lhsn)))
        {
            rsetregister(V->ralloc, dst.val_register, TValue(lhsn->val_number < rhsn->val_number));
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(checktable(V, *lhsn)))
        {
            TValue metamethod = getmetamethod(V, *lhsn, OpCode::LESS);

            tspush(V->stack, *lhsn);
            tspush(V->stack, *lhsn);
            call(V, metamethod, 2);

            StkVal val = tspop(V->stack);

            rsetregister(V->ralloc, dst.val_register, val);
            VM_NEXT();
        }

        VM_NEXT();
    }
    case OpCode::GREATER:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsn = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsn = rgetregister(V->ralloc, rhs.val_register);

        if (VIA_LIKELY(checknumber(V, *lhsn)))
        {
            rsetregister(V->ralloc, dst.val_register, TValue(lhsn->val_number > rhsn->val_number));
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(checktable(V, *lhsn)))
        {
            TValue metamethod = getmetamethod(V, *lhsn, OpCode::LESS);

            tspush(V->stack, *lhsn);
            tspush(V->stack, *rhsn);
            call(V, metamethod, 2);

            StkVal val = tspop(V->stack);

            rsetregister(V->ralloc, dst.val_register, val);
            VM_NEXT();
        }

        VM_NEXT();
    }
    case OpCode::LESSOREQUAL:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsn = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsn = rgetregister(V->ralloc, rhs.val_register);

        if (VIA_LIKELY(checknumber(V, *lhsn)))
        {
            rsetregister(V->ralloc, dst.val_register, TValue(lhsn->val_number <= rhsn->val_number));
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(checktable(V, *lhsn)))
        {
            TValue metamethod = getmetamethod(V, *lhsn, OpCode::LESS);

            tspush(V->stack, *lhsn);
            tspush(V->stack, *rhsn);
            call(V, metamethod, 2);

            StkVal val = tspop(V->stack);

            rsetregister(V->ralloc, dst.val_register, val);
            VM_NEXT();
        }

        VM_NEXT();
    }
    case OpCode::GREATEROREQUAL:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsn = rgetregister(V->ralloc, lhs.val_register);
        TValue *rhsn = rgetregister(V->ralloc, rhs.val_register);

        if (VIA_LIKELY(checknumber(V, *lhsn)))
        {
            rsetregister(V->ralloc, dst.val_register, TValue(lhsn->val_number >= rhsn->val_number));
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(checktable(V, *lhsn)))
        {
            TValue metamethod = getmetamethod(V, *lhsn, OpCode::LESS);

            tspush(V->stack, *lhsn);
            tspush(V->stack, *rhsn);
            call(V, metamethod, 2);

            StkVal val = tspop(V->stack);

            rsetregister(V->ralloc, dst.val_register, val);
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

    case OpCode::JUMP:
    {
        Operand offset = V->ip->operand1;
        V->ip += static_cast<JmpOffset>(offset.val_number);
        VM_NEXT();
    }

    case OpCode::JUMPIFNOT:
    case OpCode::JUMPIF:
    {
        Operand condr = V->ip->operand1;
        Operand offset = V->ip->operand2;

        TValue cond = *rgetregister(V->ralloc, condr.val_register);
        // We don't need to save the return value because this function modifies `cond`
        tonumber(V, cond);
        if (V->ip->op == OpCode::JUMPIFNOT ? (cond.val_number != 0) : (cond.val_number == 0))
            V->ip += static_cast<JmpOffset>(offset.val_number);

        VM_NEXT();
    }

    case OpCode::JUMPIFEQUAL:
    case OpCode::JUMPIFNOTEQUAL:
    {
        Operand condlr = V->ip->operand1;
        Operand condrr = V->ip->operand2;
        Operand offset = V->ip->operand3;

        bool cond = compareregisters(V, condlr.val_register, condrr.val_register);
        if (V->ip->op == OpCode::JUMPIFEQUAL ? cond : !cond)
            V->ip += static_cast<JmpOffset>(offset.val_number);

        VM_NEXT();
    }

    case OpCode::JUMPIFLESS:
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

    case OpCode::JUMPIFGREATER:
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

    case OpCode::JUMPIFLESSOREQUAL:
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

    case OpCode::JUMPIFGREATEROREQUAL:
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

        pushval(V, *obj); // Push self
        // Call function, with [argc + 1] to account for the self argument
        nativecall(V, func->val_function, argc + 1);
        VM_NEXT();
    }

    case OpCode::RETURN:
    {
        Operand retcv = V->ip->operand1;
        CallArgc retc = static_cast<CallArgc>(retcv.val_number);

        nativeret(V, retc);
        VM_NEXT();
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
        settable(V, T, key, val);
        VM_NEXT();
    }

    case OpCode::LENSTRING:
    {
        Operand rdst = V->ip->operand1;
        Operand objr = V->ip->operand2;

        TValue *val = rgetregister(V->ralloc, objr.val_register);
        TNumber len = static_cast<TNumber>(val->val_string->len);

        rsetregister(V->ralloc, rdst.val_register, TValue(len));
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

    case OpCode::GETSTRING:
    {
        Operand dst = V->ip->operand1;
        Operand str = V->ip->operand2;
        Operand idx = V->ip->operand3;

        TValue *str_val = rgetregister(V->ralloc, str.val_register);
        TValue *idx_val = rgetregister(V->ralloc, idx.val_register);

        VM_ASSERT(checkstring(V, *idx_val), std::format("Attempt to subscript string with {}", idx_val->type));

        size_t index = idx_val->val_number;
        if (VIA_UNLIKELY(index > str_val->val_string->len))
            rsetregister(V->ralloc, dst.val_register, TValue());
    }

    default:
        setexitdata(V, 1, std::format("Unknown opcode {:x}", static_cast<uint8_t>(V->ip->op)));
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
