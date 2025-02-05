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
        if (!CHECK_JUMP_ADDRESS(V->ip + 1)) \
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
            setexitdata(V, 1, std::format("VM_ASSERT(): {}\n  file: {}\n  line: {}", (message), __FILE__, __LINE__)); \
            VM_EXIT(); \
        } \
    }

// Silent VM assertion
// Terminates VM if the assertion fails
// Does not contain additional information
#define VM_ASSERT_SILENT(cond, message) \
    { \
        if (!(cond)) \
        { \
            setexitdata(V, 1, message); \
            VM_EXIT(); \
        } \
    }

namespace via
{

// Starts VM execution cycle by altering it's state and "iterating" over the instruction pipeline.
void execute(State *VIA_RESTRICT V)
{
    VIA_ASSERT(V->tstate == ThreadState::PAUSED, "execute(): must be called on paused thread");
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
        State *sstate = V->sstate;
        delete V;
        V = sstate;
        V->restorestate = false;
        V->sstate = nullptr;
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
        VIA_UNLIKELY(CHECK_JUMP_ADDRESS(V->ip)),
        std::format(
            "Instruction pointer out of bounds (ip={}, ihp={}, ibp={})",
            reinterpret_cast<const void *>(V->ip),
            reinterpret_cast<const void *>(V->ihp),
            reinterpret_cast<const void *>(V->ibp)
        )
    );
#endif

    if (VIA_UNLIKELY(V->yield))
    {
        long long milliseconds = V->yieldfor / 1000;
        std::chrono::milliseconds dur(milliseconds);
        std::this_thread::sleep_for(dur);
        V->yield = false;
    }

    // No LOAD protocol is present here.
    // This is because the LOAD protocol is embedded into VM_NEXT.
    switch (V->ip->op)
    {
    case OpCode::NOP:
        VM_NEXT();

    case OpCode::ADD:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = getregister(V, lhs.val_register);
        TValue *rhs_val = getregister(V, rhs.val_register);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val->val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::ADD);
            push(V, *lhs_val); // Push self
            push(V, *rhs_val); // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::ADDK:
    {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        size_t const_idx = idx.val_number;
        TValue *lhs_val = getregister(V, lhs.val_register);
        const TValue &rhs_val = V->G->ktable->at(const_idx);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::ADD);
            push(V, *lhs_val); // Push self
            push(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::ADDI:
    {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = getregister(V, lhs.val_register);
        TValue rhs_val(imm);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::ADD);
            push(V, *lhs_val); // Push self
            push(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::SUB:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = getregister(V, lhs.val_register);
        TValue *rhs_val = getregister(V, rhs.val_register);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val->val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::SUB);
            push(V, *lhs_val); // Push self
            push(V, *rhs_val); // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::SUBK:
    {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        size_t const_idx = idx.val_number;
        TValue *lhs_val = getregister(V, lhs.val_register);
        const TValue &rhs_val = V->G->ktable->at(const_idx);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::SUB);
            push(V, *lhs_val); // Push self
            push(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::SUBI:
    {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = getregister(V, lhs.val_register);
        TValue rhs_val(imm);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::SUB);
            push(V, *lhs_val); // Push self
            push(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::MUL:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = getregister(V, lhs.val_register);
        TValue *rhs_val = getregister(V, rhs.val_register);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val->val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::MUL);
            push(V, *lhs_val); // Push self
            push(V, *rhs_val); // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::MULK:
    {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        size_t const_idx = idx.val_number;
        TValue *lhs_val = getregister(V, lhs.val_register);
        const TValue &rhs_val = V->G->ktable->at(const_idx);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::MUL);
            push(V, *lhs_val); // Push self
            push(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::MULI:
    {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = getregister(V, lhs.val_register);
        TValue rhs_val(imm);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::MUL);
            push(V, *lhs_val); // Push self
            push(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::DIV:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = getregister(V, lhs.val_register);
        TValue *rhs_val = getregister(V, rhs.val_register);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val->val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::DIV);
            push(V, *lhs_val); // Push self
            push(V, *rhs_val); // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::DIVK:
    {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        size_t const_idx = idx.val_number;
        TValue *lhs_val = getregister(V, lhs.val_register);
        const TValue &rhs_val = V->G->ktable->at(const_idx);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::DIV);
            push(V, *lhs_val); // Push self
            push(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::DIVI:
    {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = getregister(V, lhs.val_register);
        TValue rhs_val(imm);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::DIV);
            push(V, *lhs_val); // Push self
            push(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::POW:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = getregister(V, lhs.val_register);
        TValue *rhs_val = getregister(V, rhs.val_register);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val->val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::POW);
            push(V, *lhs_val); // Push self
            push(V, *rhs_val); // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::POWK:
    {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        size_t const_idx = idx.val_number;
        TValue *lhs_val = getregister(V, lhs.val_register);
        const TValue &rhs_val = V->G->ktable->at(const_idx);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::POW);
            push(V, *lhs_val); // Push self
            push(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::POWI:
    {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = getregister(V, lhs.val_register);
        TValue rhs_val(imm);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::POW);
            push(V, *lhs_val); // Push self
            push(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::MOD:
    {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = getregister(V, lhs.val_register);
        TValue *rhs_val = getregister(V, rhs.val_register);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val->val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::MOD);
            push(V, *lhs_val); // Push self
            push(V, *rhs_val); // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::MODK:
    {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        size_t const_idx = idx.val_number;
        TValue *lhs_val = getregister(V, lhs.val_register);
        const TValue &rhs_val = V->G->ktable->at(const_idx);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::MOD);
            push(V, *lhs_val); // Push self
            push(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::MODI:
    {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = getregister(V, lhs.val_register);
        TValue rhs_val(imm);

        // Fast-path: lvalue is a number
        if (VIA_LIKELY(checknumber(V, *lhs_val)))
            lhs_val->val_number += rhs_val.val_number;
        else if (checktable(V, *lhs_val))
        {
            const TValue &metamethod = getmetamethod(V, *lhs_val, OpCode::MOD);
            push(V, *lhs_val); // Push self
            push(V, rhs_val);  // Push other
            call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::MOVE:
    {
        Operand rdst = V->ip->operand1;
        Operand rsrc = V->ip->operand2;
        TValue *src_val = getregister(V, rsrc.val_register);

        setregister(V, rdst.val_register, *src_val);
        VM_NEXT();
    }

    case OpCode::LOADK:
    {
        Operand dst = V->ip->operand1;
        Operand idx = V->ip->operand2;
        kTable::size_type kid = idx.val_number;

        // Check if the kId is valid
        VM_ASSERT(kid > V->G->ktable->size(), "LOADK: Invalid constant index (out of range)");

        const TValue &kval = V->G->ktable->at(kid);

        setregister(V, dst.val_register, kval);
        VM_NEXT();
    }

    case OpCode::LOADNIL:
    {
        Operand dst = V->ip->operand1;

        setregister(V, dst.val_register, nil);
        VM_NEXT();
    }

    case OpCode::LOADTABLE:
    {
        Operand dst = V->ip->operand1;
        TValue ttable(new TTable());

        setregister(V, dst.val_register, ttable);
        VM_NEXT();
    }

    case OpCode::LOADBOOL:
    {
        Operand dst = V->ip->operand1;
        Operand val = V->ip->operand2;
        TValue tbool(val.val_boolean);

        setregister(V, dst.val_register, tbool);
        VM_NEXT();
    }

    case OpCode::LOADNUMBER:
    {
        Operand dst = V->ip->operand1;
        Operand val = V->ip->operand2;
        TValue tnumber(val.val_number);

        setregister(V, dst.val_register, tnumber);
        VM_NEXT();
    }

    case OpCode::LOADSTRING:
    {
        Operand dst = V->ip->operand1;
        Operand val = V->ip->operand2;
        TValue tstring(val.val_string);

        setregister(V, dst.val_register, tstring);
        VM_NEXT();
    }

    case OpCode::LOADFUNCTION:
    {
        Operand dst = V->ip->operand1;
        TFunction *func = new TFunction(V, "<anonymous>", V->ip, V->frame, {}, false, false);
        TValue val(func);

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

        setregister(V, dst.val_register, val);
        // Dispatch instead of invoking VM_NEXT
        VM_DISPATCH();
    }

    case OpCode::PUSH:
    {
        Operand src = V->ip->operand1;
        TValue *val = getregister(V, src.val_register);

        push(V, *val);
        VM_NEXT();
    }

    case OpCode::POP:
    {
        Operand dst = V->ip->operand1;
        TValue val = pop(V);

        setregister(V, dst.val_register, val);
        VM_NEXT();
    }

    case OpCode::GETSTACK:
    {
        Operand dst = V->ip->operand1;
        Operand off = V->ip->operand2;

        StkPos stack_offset = static_cast<StkPos>(off.val_number);
        const TValue &val = *(V->sbp + stack_offset);

        setregister(V, dst.val_register, val);
        VM_NEXT();
    }

    case OpCode::SETSTACK:
    {
        Operand src = V->ip->operand1;
        Operand off = V->ip->operand2;

        StkPos stack_offset = static_cast<StkPos>(off.val_number);
        TValue *val = getregister(V, src.val_register);

        *(V->sbp + stack_offset) = std::move(*val);
        VM_NEXT();
    }

    case OpCode::GETARGUMENT:
    {
        Operand dst = V->ip->operand1;
        Operand off = V->ip->operand2;

        LocalId offv = static_cast<LocalId>(off.val_number);
        const TValue &val = getargument(V, offv);

        setregister(V, dst.val_register, val);
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
            TValue &val = *getregister(V, lhs.val_register);
            lhsn = std::move(val);
            lhs_reg = true;
        }
        else
            lhsn = TValue(lhs);

        if (VIA_UNLIKELY(rhs.type == OperandType::Register))
        {
            TValue &val = *getregister(V, rhs.val_register);
            rhsn = std::move(val);
            rhs_reg = true;
        }
        else
            rhsn = TValue(rhs);

        if (lhs_reg && rhs_reg)
        {
            TValue val(!compareregisters(V, lhs.val_register, rhs.val_register));
            setregister(V, dst.val_register, val);
        }
        else if (lhs_reg)
        {
            TValue val(!compare(V, *getregister(V, lhs.val_register), rhsn));
            setregister(V, dst.val_register, val);
        }
        else if (rhs_reg)
        {
            TValue val(!compare(V, *getregister(V, rhs.val_register), lhsn));
            setregister(V, dst.val_register, val);
        }
        else
        {
            TValue val(!compare(V, lhsn, rhsn));
            setregister(V, dst.val_register, val);
        }

        VM_NEXT();
    }
    case OpCode::LESS:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsn = getregister(V, lhs.val_register);
        TValue *rhsn = getregister(V, rhs.val_register);

        if (VIA_LIKELY(checknumber(V, *lhsn)))
        {
            TValue val(lhsn->val_number < rhsn->val_number);
            setregister(V, dst.val_register, val);
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(checktable(V, *lhsn)))
        {
            const TValue &metamethod = getmetamethod(V, *lhsn, OpCode::LESS);

            push(V, *lhsn);
            push(V, *lhsn);
            call(V, metamethod, 2);

            TValue val = pop(V);

            setregister(V, dst.val_register, val);
            VM_NEXT();
        }

        VM_NEXT();
    }
    case OpCode::GREATER:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsn = getregister(V, lhs.val_register);
        TValue *rhsn = getregister(V, rhs.val_register);

        if (VIA_LIKELY(checknumber(V, *lhsn)))
        {
            TValue val(lhsn->val_number > rhsn->val_number);
            setregister(V, dst.val_register, val);
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(checktable(V, *lhsn)))
        {
            const TValue &metamethod = getmetamethod(V, *lhsn, OpCode::LESS);

            push(V, *lhsn);
            push(V, *rhsn);
            call(V, metamethod, 2);

            TValue val = pop(V);

            setregister(V, dst.val_register, val);
            VM_NEXT();
        }

        VM_NEXT();
    }
    case OpCode::LESSOREQUAL:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsn = getregister(V, lhs.val_register);
        TValue *rhsn = getregister(V, rhs.val_register);

        if (VIA_LIKELY(checknumber(V, *lhsn)))
        {
            TValue val(lhsn->val_number <= rhsn->val_number);
            setregister(V, dst.val_register, val);
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(checktable(V, *lhsn)))
        {
            const TValue &metamethod = getmetamethod(V, *lhsn, OpCode::LESS);

            push(V, *lhsn);
            push(V, *rhsn);
            call(V, metamethod, 2);

            TValue val = pop(V);

            setregister(V, dst.val_register, val);
            VM_NEXT();
        }

        VM_NEXT();
    }
    case OpCode::GREATEROREQUAL:
    {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsn = getregister(V, lhs.val_register);
        TValue *rhsn = getregister(V, rhs.val_register);

        if (VIA_LIKELY(checknumber(V, *lhsn)))
        {
            TValue val(lhsn->val_number >= rhsn->val_number);
            setregister(V, dst.val_register, val);
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(checktable(V, *lhsn)))
        {
            const TValue &metamethod = getmetamethod(V, *lhsn, OpCode::LESS);

            push(V, *lhsn);
            push(V, *rhsn);
            call(V, metamethod, 2);

            TValue val = pop(V);

            setregister(V, dst.val_register, val);
            VM_NEXT();
        }

        VM_NEXT();
    }

    case OpCode::EXIT:
    {
        Operand rcode = V->ip->operand1;

        TValue *code = getregister(V, rcode.val_register);
        TNumber ecode = tonumber(V, *code).val_number;

        setexitdata(V, ecode, "VM exited by user");
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

        TValue &cond = *getregister(V, condr.val_register);
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

        TValue *lhs = getregister(V, condlr.val_register);
        TValue *rhs = getregister(V, condrr.val_register);

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

        TValue *lhs = getregister(V, condlr.val_register);
        TValue *rhs = getregister(V, condrr.val_register);

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

        TValue *lhs = getregister(V, condlr.val_register);
        TValue *rhs = getregister(V, condrr.val_register);

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

        TValue *lhs = getregister(V, condlr.val_register);
        TValue *rhs = getregister(V, condrr.val_register);

        bool cond = lhs->val_number >= rhs->val_number;
        if (cond)
            V->ip += static_cast<JmpOffset>(offset.val_number);

        VM_NEXT();
    }

    case OpCode::CALL:
    {
        Operand rfn = V->ip->operand1;
        Operand argco = V->ip->operand2;
        size_t argc = static_cast<size_t>(argco.val_number);
        // Call function
        call(V, *getregister(V, rfn.val_register), argc);
        VM_NEXT();
    }
    case OpCode::EXTERNCALL:
    {
        Operand rfn = V->ip->operand1;
        Operand argco = V->ip->operand2;
        size_t argc = static_cast<size_t>(argco.val_number);
        TValue *cfunc = getregister(V, rfn.val_register);

        // Call function
        externcall(V, cfunc->val_cfunction, argc);
        VM_NEXT();
    }
    case OpCode::NATIVECALL:
    {
        Operand rfn = V->ip->operand1;
        Operand argco = V->ip->operand2;
        size_t argc = static_cast<size_t>(argco.val_number);
        TValue *func = getregister(V, rfn.val_register);

        // Call function
        nativecall(V, func->val_function, argc);
        VM_NEXT();
    }
    case OpCode::METHODCALL:
    {
        Operand robj = V->ip->operand1;
        Operand rfn = V->ip->operand2;
        Operand argco = V->ip->operand3;

        size_t argc = static_cast<size_t>(argco.val_number);
        TValue *func = getregister(V, rfn.val_register);
        TValue *obj = getregister(V, robj.val_register);

        push(V, *obj); // Push self
        // Call function, with [argc + 1] to account for the self argument
        nativecall(V, func->val_function, argc + 1);
        VM_NEXT();
    }

    case OpCode::RETURN:
    {
        Operand retcv = V->ip->operand1;
        size_t retc = static_cast<size_t>(retcv.val_number);

        nativeret(V, retc);
        VM_NEXT();
    }

    case OpCode::GETTABLE:
    {
        Operand rdst = V->ip->operand1;
        Operand rtbl = V->ip->operand2;
        Operand ridx = V->ip->operand3;

        TValue &tbl = *getregister(V, rtbl.val_register);
        TValue &idx = *getregister(V, ridx.val_register);

        // Get table key based on the index type (string or number)
        TableKey key = checkstring(V, idx) ? idx.val_string->hash : idx.val_number;
        const TValue &index = gettable(V, tbl.val_table, key, true);

        setregister(V, rdst.val_register, index);
        VM_NEXT();
    }

    case OpCode::SETTABLE:
    {
        Operand rsrc = V->ip->operand1;
        Operand rtbl = V->ip->operand2;
        Operand ridx = V->ip->operand3;

        TValue val;
        TValue &tbl = *getregister(V, rtbl.val_register);
        TValue &idx = *getregister(V, ridx.val_register);

        // Get table key based on the index type (string or number)
        TableKey key = checkstring(V, idx) ? idx.val_string->hash : static_cast<Hash>(idx.val_number);
        // Slow-path: the value is stored in a register, load it
        if (VIA_UNLIKELY(rsrc.type == OperandType::Register))
        {
            TValue &temp = *getregister(V, rsrc.val_register);
            val = std::move(temp);
        }
        else
            val = TValue(rsrc);

        // Set the table index
        TTable *T = tbl.val_table;
        settable(V, T, key, val);
        VM_NEXT();
    }

    case OpCode::NEXTTABLE:
    {
        static std::unordered_map<void *, TableKey> next_table;

        Operand dst = V->ip->operand1;
        Operand valr = V->ip->operand2;

        TValue *val = getregister(V, valr.val_register);
        void *ptr = topointer(V, *val);
        TableKey key = 0;

        // Look for the current key in next_table and increment it if found
        auto it = next_table.find(ptr);
        if (it != next_table.end())
            key = ++it->second;
        else
            next_table[ptr] = 0;

        auto table_map = val->val_table->data;
        auto field_it = table_map.find(key);

        if (field_it != table_map.end())
            // If the key is found, use the corresponding value
            setregister(V, dst.val_register, field_it->second);
        else // If not found, set the value to a default (e.g., nil)
            setregister(V, dst.val_register, nil);

        VM_NEXT();
    }

    case OpCode::LENTABLE:
    {
        Operand dst = V->ip->operand1;
        Operand tblr = V->ip->operand2;

        TValue *val = getregister(V, tblr.val_register);
        TNumber size = static_cast<TNumber>(val->val_table->data.size());
        TValue val_size(size);

        setregister(V, dst.val_register, val_size);
        VM_NEXT();
    }

    case OpCode::LENSTRING:
    {
        Operand rdst = V->ip->operand1;
        Operand objr = V->ip->operand2;

        TValue *val = getregister(V, objr.val_register);
        TNumber len = static_cast<TNumber>(val->val_string->len);
        TValue val_len(len);

        setregister(V, rdst.val_register, val_len);
        VM_NEXT();
    }

    case OpCode::GETSTRING:
    {
        Operand dst = V->ip->operand1;
        Operand str = V->ip->operand2;
        Operand idx = V->ip->operand3;

        TValue *str_val = getregister(V, str.val_register);
        TValue *idx_val = getregister(V, idx.val_register);

        VM_ASSERT_SILENT(checkstring(V, *idx_val), std::format("Attempt to subscript string with {}", idx_val->type));

        size_t index = idx_val->val_number;
        if (VIA_UNLIKELY(index > str_val->val_string->len))
            setregister(V, dst.val_register, nil);

        VM_NEXT();
    }

    case OpCode::LEN:
    {
        Operand rdst = V->ip->operand1;
        Operand objr = V->ip->operand2;

        TValue *val = getregister(V, objr.val_register);
        TValue len = via::len(V, *val);

        setregister(V, rdst.val_register, len);
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
        TValue type = typeofv(V, *val);

        setregister(V, rdst.val_register, type);
        VM_NEXT();
    }

    default:
        // This should be unreachable, however, since some
        // opcodes are yet to be implemented it is not marked as so.
        VM_ASSERT(false, "Unknown opcode");
    }
}

exit:;
}

// Permanently kills the thread. Does not clean up the state object.
void killthread(State *VIA_RESTRICT V)
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
void pausethread(State *VIA_RESTRICT V)
{
    V->tstate = ThreadState::PAUSED;
    // Save the VM state to restore it when paused
    savestate(V);
}

} // namespace via
