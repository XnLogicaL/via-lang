/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "execute.h"
#include "vmapi.h"
#include "chunk.h"
#include "common.h"
#include "state.h"
#include "types.h"

// Define the hot path threshold for the instruction dispatch loop
// How many times a chunk needs to be executed before being flagged as "hot"
#ifndef VIA_HOTPATH_THRESHOLD
    #define VIA_HOTPATH_THRESHOLD 64
#endif

#define VM_ERROR(message) \
    { \
        __set_error_state(V, message); \
        V->sig_error.fire(); \
        goto exit; \
    }

#define VM_FATAL(message) \
    { \
        std::cerr << "VM terminated with message: " << message << '\n'; \
        V->sig_fatal.fire(); \
        std::abort(); \
    }

#define VM_ASSERT(cond, message) \
    if (cond) \
        VM_ERROR(message);

// Macro for loading the next instruction
// Has bound checks
#define VM_LOAD() \
    { \
        if (!CHECK_JUMP_ADDRESS(V->ip + 1)) \
            VM_FATAL("illegal instruction access"); \
        V->ip++; \
    }

// Macro that "signals" the VM has completed an execution cycle
#define VM_NEXT() \
    { \
        VM_LOAD(); \
        goto dispatch; \
    }

namespace via {

// Starts VM execution cycle by altering it's state and "iterating" over the instruction pipeline.
void execute(State *VIA_RESTRICT V)
{
    using namespace impl;

    VIA_ASSERT(V->tstate == ThreadState::PAUSED, "via::execute must be called on inactive thread");
    V->tstate = ThreadState::RUNNING;

    goto dispatch;

dispatch: {
    // Check for errors and attempt handling them.
    // The __handle_error function works by unwinding the stack until either
    // hitting a stack frame flagged as error handler, or,
    // the root stack frame, and the root stack frame cannot be an error handler under any circumstances.
    // Therefore the error will act as a fatal error, being automatically thrown by __handle_error,
    // along with a callstack and debug information.
    if (__has_error(V) && !__handle_error(V)) {
        goto exit;
    }

    // Abort is second priority due to verbosity.
    if (V->abort) {
        V->sig_abort.fire();
        goto exit;
    }

    switch (V->ip->op) {
    case OpCode::NOP:
        VM_NEXT();

    case OpCode::ADD: {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = __get_register(V, lhs.val_register);
        TValue *rhs_val = __get_register(V, rhs.val_register);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val->val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::ADD);
            __push(V, *lhs_val); // Push self
            __push(V, *rhs_val); // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::ADDK: {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        std::lock_guard<std::shared_mutex> lock(V->G->ktable_mutex);

        size_t const_idx = idx.val_number;
        TValue *lhs_val = __get_register(V, lhs.val_register);
        const TValue &rhs_val = V->G->ktable.at(const_idx);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val.val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::ADD);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::ADDI: {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = __get_register(V, lhs.val_register);
        TValue rhs_val(imm);

        // Fast-path: lhs value is a number
        if VIA_LIKELY (check_number(*lhs_val)) {
            lhs_val->val_number += rhs_val.val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::ADD);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::SUB: {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = __get_register(V, lhs.val_register);
        TValue *rhs_val = __get_register(V, rhs.val_register);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val->val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::SUB);
            __push(V, *lhs_val); // Push self
            __push(V, *rhs_val); // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::SUBK: {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        std::lock_guard<std::shared_mutex> lock(V->G->ktable_mutex);

        size_t const_idx = idx.val_number;
        TValue *lhs_val = __get_register(V, lhs.val_register);
        const TValue &rhs_val = V->G->ktable.at(const_idx);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val.val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::SUB);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::SUBI: {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = __get_register(V, lhs.val_register);
        TValue rhs_val(imm);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val.val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::SUB);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::MUL: {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = __get_register(V, lhs.val_register);
        TValue *rhs_val = __get_register(V, rhs.val_register);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val->val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::MUL);
            __push(V, *lhs_val); // Push self
            __push(V, *rhs_val); // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::MULK: {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        std::lock_guard<std::shared_mutex> lock(V->G->ktable_mutex);

        size_t const_idx = idx.val_number;
        TValue *lhs_val = __get_register(V, lhs.val_register);
        const TValue &rhs_val = V->G->ktable.at(const_idx);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val.val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::MUL);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::MULI: {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = __get_register(V, lhs.val_register);
        TValue rhs_val(imm);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val.val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::MUL);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::DIV: {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = __get_register(V, lhs.val_register);
        TValue *rhs_val = __get_register(V, rhs.val_register);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val->val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::DIV);
            __push(V, *lhs_val); // Push self
            __push(V, *rhs_val); // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::DIVK: {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        std::lock_guard<std::shared_mutex> lock(V->G->ktable_mutex);

        size_t const_idx = idx.val_number;
        TValue *lhs_val = __get_register(V, lhs.val_register);
        const TValue &rhs_val = V->G->ktable.at(const_idx);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val.val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::DIV);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::DIVI: {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = __get_register(V, lhs.val_register);
        TValue rhs_val(imm);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val.val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::DIV);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::POW: {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = __get_register(V, lhs.val_register);
        TValue *rhs_val = __get_register(V, rhs.val_register);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val->val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::POW);
            __push(V, *lhs_val); // Push self
            __push(V, *rhs_val); // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::POWK: {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        std::lock_guard<std::shared_mutex> lock(V->G->ktable_mutex);

        size_t const_idx = idx.val_number;
        TValue *lhs_val = __get_register(V, lhs.val_register);
        const TValue &rhs_val = V->G->ktable.at(const_idx);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val.val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::POW);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::POWI: {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = __get_register(V, lhs.val_register);
        TValue rhs_val(imm);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val.val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::POW);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::MOD: {
        Operand lhs = V->ip->operand1;
        Operand rhs = V->ip->operand2;

        TValue *lhs_val = __get_register(V, lhs.val_register);
        TValue *rhs_val = __get_register(V, rhs.val_register);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val->val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::MOD);
            __push(V, *lhs_val); // Push self
            __push(V, *rhs_val); // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::MODK: {
        Operand lhs = V->ip->operand1;
        Operand idx = V->ip->operand2;

        std::lock_guard<std::shared_mutex> lock(V->G->ktable_mutex);

        size_t const_idx = idx.val_number;
        TValue *lhs_val = __get_register(V, lhs.val_register);
        const TValue &rhs_val = V->G->ktable.at(const_idx);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val.val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::MOD);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }
    case OpCode::MODI: {
        Operand lhs = V->ip->operand1;
        Operand imm = V->ip->operand2;

        TValue *lhs_val = __get_register(V, lhs.val_register);
        TValue rhs_val(imm);

        // Fast-path: lhs value is a number
        if (VIA_LIKELY(check_number(*lhs_val))) {
            lhs_val->val_number += rhs_val.val_number;
        }
        else if (check_table(*lhs_val)) {
            const TValue &metamethod = __get_metamethod(*lhs_val, OpCode::MOD);
            __push(V, *lhs_val); // Push self
            __push(V, rhs_val);  // Push other
            __call(V, metamethod, 2);
        }

        VM_NEXT();
    }

    case OpCode::MOVE: {
        Operand rdst = V->ip->operand1;
        Operand rsrc = V->ip->operand2;
        TValue *src_val = __get_register(V, rsrc.val_register);

        __set_register(V, rdst.val_register, *src_val);
        VM_NEXT();
    }

    case OpCode::LOADK: {
        Operand dst = V->ip->operand1;
        Operand idx = V->ip->operand2;
        U64 kid = idx.val_number;

        std::lock_guard<std::shared_mutex> lock(V->G->ktable_mutex);

        // Check if the kId is valid
        if (kid > V->G->ktable.size()) {
            VM_FATAL("invalid constant index");
        }

        const TValue &kval = V->G->ktable.at(kid);

        __set_register(V, dst.val_register, kval);
        VM_NEXT();
    }

    case OpCode::LOADNIL: {
        Operand dst = V->ip->operand1;

        __set_register(V, dst.val_register, _Nil);
        VM_NEXT();
    }

    case OpCode::LOADTABLE: {
        Operand dst = V->ip->operand1;
        TValue ttable(new TTable());

        __set_register(V, dst.val_register, ttable);
        VM_NEXT();
    }

    case OpCode::LOADBOOL: {
        Operand dst = V->ip->operand1;
        Operand val = V->ip->operand2;
        TValue tbool(val.val_boolean);

        __set_register(V, dst.val_register, tbool);
        VM_NEXT();
    }

    case OpCode::LOADNUMBER: {
        Operand dst = V->ip->operand1;
        Operand val = V->ip->operand2;
        TValue tnumber(val.val_number);

        __set_register(V, dst.val_register, tnumber);
        VM_NEXT();
    }

    case OpCode::LOADSTRING: {
        Operand dst = V->ip->operand1;
        Operand val = V->ip->operand2;
        TValue tstring(val.val_string);

        __set_register(V, dst.val_register, tstring);
        VM_NEXT();
    }

    case OpCode::LOADFUNCTION: {
        Operand dst = V->ip->operand1;
        TFunction *func = new TFunction(V, "<anonymous>", V->ip, V->frame, {}, false, false);
        TValue val(func);

        while (V->ip < V->ibp) {
            if (V->ip->op == OpCode::RETURN) {
                V->ip++;
                break;
            }
            // Copy the instruction and insert into the function object
            func->bytecode.push_back(*(V->ip++));
        }

        __set_register(V, dst.val_register, val);
        // Dispatch instead of invoking VM_NEXT
        goto dispatch;
    }

    case OpCode::PUSH: {
        Operand src = V->ip->operand1;
        TValue *val = __get_register(V, src.val_register);

        __push(V, *val);
        VM_NEXT();
    }

    case OpCode::PUSHK: {
        Operand const_idx = V->ip->operand1;

        std::lock_guard<std::shared_mutex> lock(V->G->ktable_mutex);

        size_t const_id = const_idx.val_number;
        const TValue &constant = V->G->ktable.at(const_id);

        __push(V, constant);
        VM_NEXT();
    }

    case OpCode::PUSHI: {
        Operand immx = V->ip->operand1;
        TValue val(immx);

        __push(V, val);
        VM_NEXT();
    }

    case OpCode::POP: {
        Operand dst = V->ip->operand1;
        TValue val = __pop(V);

        __set_register(V, dst.val_register, val);
        VM_NEXT();
    }

    case OpCode::GETSTACK: {
        Operand dst = V->ip->operand1;
        Operand off = V->ip->operand2;

        StkPos stack_offset = static_cast<StkPos>(off.val_number);
        const TValue &val = *(V->sbp + stack_offset);

        __set_register(V, dst.val_register, val);
        VM_NEXT();
    }

    case OpCode::SETSTACK: {
        Operand src = V->ip->operand1;
        Operand off = V->ip->operand2;

        StkPos stack_offset = static_cast<StkPos>(off.val_number);
        TValue *val = __get_register(V, src.val_register);

        *(V->sbp + stack_offset) = std::move(*val);
        VM_NEXT();
    }

    case OpCode::GETARGUMENT: {
        Operand dst = V->ip->operand1;
        Operand off = V->ip->operand2;

        LocalId offv = static_cast<LocalId>(off.val_number);
        const TValue &val = __get_argument(V, offv);

        __set_register(V, dst.val_register, val);
        VM_NEXT();
    }

    case OpCode::GETGLOBAL: {
        Operand dst = V->ip->operand1;
        Operand glb_idx = V->ip->operand2;

        kGlobId glb_id(glb_idx.val_string);
        const TValue &global = __get_global(V, glb_id);

        __set_register(V, dst.val_register, global);
        VM_NEXT();
    }

    case OpCode::NOTEQUAL: {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue lhsn, rhsn;

        bool lhs_reg = false;
        bool rhs_reg = false;

        if (VIA_UNLIKELY(lhs.type == OperandType::Register)) {
            TValue &val = *__get_register(V, lhs.val_register);
            lhsn = std::move(val);
            lhs_reg = true;
        }
        else {
            lhsn = TValue(lhs);
        }

        if (VIA_UNLIKELY(rhs.type == OperandType::Register)) {
            TValue &val = *__get_register(V, rhs.val_register);
            rhsn = std::move(val);
            rhs_reg = true;
        }
        else {
            rhsn = TValue(rhs);
        }

        if (lhs_reg && rhs_reg) {
            TValue val(!__compare_registers(V, lhs.val_register, rhs.val_register));
            __set_register(V, dst.val_register, val);
        }
        else if (lhs_reg) {
            TValue val(!__compare(*__get_register(V, lhs.val_register), rhsn));
            __set_register(V, dst.val_register, val);
        }
        else if (rhs_reg) {
            TValue val(!__compare(*__get_register(V, rhs.val_register), lhsn));
            __set_register(V, dst.val_register, val);
        }
        else {
            TValue val(!__compare(lhsn, rhsn));
            __set_register(V, dst.val_register, val);
        }

        VM_NEXT();
    }
    case OpCode::LESS: {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsn = __get_register(V, lhs.val_register);
        TValue *rhsn = __get_register(V, rhs.val_register);

        if (VIA_LIKELY(check_number(*lhsn))) {
            TValue val(lhsn->val_number < rhsn->val_number);
            __set_register(V, dst.val_register, val);
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(check_table(*lhsn))) {
            const TValue &metamethod = __get_metamethod(*lhsn, OpCode::LESS);

            __push(V, *lhsn);
            __push(V, *lhsn);
            __call(V, metamethod, 2);

            TValue val = __pop(V);

            __set_register(V, dst.val_register, val);
            VM_NEXT();
        }

        VM_NEXT();
    }
    case OpCode::GREATER: {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsn = __get_register(V, lhs.val_register);
        TValue *rhsn = __get_register(V, rhs.val_register);

        if (VIA_LIKELY(check_number(*lhsn))) {
            TValue val(lhsn->val_number > rhsn->val_number);
            __set_register(V, dst.val_register, val);
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(check_table(*lhsn))) {
            const TValue &metamethod = __get_metamethod(*lhsn, OpCode::LESS);

            __push(V, *lhsn);
            __push(V, *rhsn);
            __call(V, metamethod, 2);

            TValue val = __pop(V);

            __set_register(V, dst.val_register, val);
            VM_NEXT();
        }

        VM_NEXT();
    }
    case OpCode::LESSOREQUAL: {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsn = __get_register(V, lhs.val_register);
        TValue *rhsn = __get_register(V, rhs.val_register);

        if (VIA_LIKELY(check_number(*lhsn))) {
            TValue val(lhsn->val_number <= rhsn->val_number);
            __set_register(V, dst.val_register, val);
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(check_table(*lhsn))) {
            const TValue &metamethod = __get_metamethod(*lhsn, OpCode::LESS);

            __push(V, *lhsn);
            __push(V, *rhsn);
            __call(V, metamethod, 2);

            TValue val = __pop(V);

            __set_register(V, dst.val_register, val);
            VM_NEXT();
        }

        VM_NEXT();
    }
    case OpCode::GREATEROREQUAL: {
        Operand dst = V->ip->operand1;
        Operand lhs = V->ip->operand2;
        Operand rhs = V->ip->operand3;

        TValue *lhsn = __get_register(V, lhs.val_register);
        TValue *rhsn = __get_register(V, rhs.val_register);

        if (VIA_LIKELY(check_number(*lhsn))) {
            TValue val(lhsn->val_number >= rhsn->val_number);
            __set_register(V, dst.val_register, val);
            VM_NEXT();
        }
        else if (VIA_UNLIKELY(check_table(*lhsn))) {
            const TValue &metamethod = __get_metamethod(*lhsn, OpCode::LESS);

            __push(V, *lhsn);
            __push(V, *rhsn);
            __call(V, metamethod, 2);

            TValue val = __pop(V);

            __set_register(V, dst.val_register, val);
            VM_NEXT();
        }

        VM_NEXT();
    }

    case OpCode::EXIT:
        goto exit;

    case OpCode::JUMP: {
        Operand offset = V->ip->operand1;
        V->ip += static_cast<JmpOffset>(offset.val_number);
        VM_NEXT();
    }

    case OpCode::JUMPIFNOT:
    case OpCode::JUMPIF: {
        Operand condr = V->ip->operand1;
        Operand offset = V->ip->operand2;

        TValue &cond = *__get_register(V, condr.val_register);
        bool cond_val = __to_cxx_bool(cond);

        if (V->ip->op == OpCode::JUMPIFNOT ? !cond_val : cond_val) {
            V->ip += static_cast<JmpOffset>(offset.val_number);
        }

        VM_NEXT();
    }

    case OpCode::JUMPIFEQUAL:
    case OpCode::JUMPIFNOTEQUAL: {
        Operand condlr = V->ip->operand1;
        Operand condrr = V->ip->operand2;
        Operand offset = V->ip->operand3;

        bool cond = __compare_registers(V, condlr.val_register, condrr.val_register);
        if (V->ip->op == OpCode::JUMPIFEQUAL ? cond : !cond) {
            V->ip += static_cast<JmpOffset>(offset.val_number);
        }

        VM_NEXT();
    }

    case OpCode::JUMPIFLESS: {
        Operand condlr = V->ip->operand1;
        Operand condrr = V->ip->operand2;
        Operand offset = V->ip->operand3;

        TValue *lhs = __get_register(V, condlr.val_register);
        TValue *rhs = __get_register(V, condrr.val_register);

        if (lhs->val_number < rhs->val_number) {
            V->ip += static_cast<JmpOffset>(offset.val_number);
        }

        VM_NEXT();
    }

    case OpCode::JUMPIFGREATER: {
        Operand condlr = V->ip->operand1;
        Operand condrr = V->ip->operand2;
        Operand offset = V->ip->operand3;

        TValue *lhs = __get_register(V, condlr.val_register);
        TValue *rhs = __get_register(V, condrr.val_register);

        if (lhs->val_number > rhs->val_number) {
            V->ip += static_cast<JmpOffset>(offset.val_number);
        }

        VM_NEXT();
    }

    case OpCode::JUMPIFLESSOREQUAL: {
        Operand condlr = V->ip->operand1;
        Operand condrr = V->ip->operand2;
        Operand offset = V->ip->operand3;

        TValue *lhs = __get_register(V, condlr.val_register);
        TValue *rhs = __get_register(V, condrr.val_register);

        if (lhs->val_number <= rhs->val_number) {
            V->ip += static_cast<JmpOffset>(offset.val_number);
        }

        VM_NEXT();
    }

    case OpCode::JUMPIFGREATEROREQUAL: {
        Operand condlr = V->ip->operand1;
        Operand condrr = V->ip->operand2;
        Operand offset = V->ip->operand3;

        TValue *lhs = __get_register(V, condlr.val_register);
        TValue *rhs = __get_register(V, condrr.val_register);

        if (lhs->val_number >= rhs->val_number) {
            V->ip += static_cast<JmpOffset>(offset.val_number);
        }

        VM_NEXT();
    }

    case OpCode::CALL: {
        Operand rfn = V->ip->operand1;
        Operand argco = V->ip->operand2;
        size_t argc = static_cast<size_t>(argco.val_number);

        TValue *fn = __get_register(V, rfn.val_register);

        // Call function
        __call(V, *fn, argc);
        VM_NEXT();
    }
    case OpCode::EXTERNCALL: {
        Operand rfn = V->ip->operand1;
        Operand argco = V->ip->operand2;
        size_t argc = static_cast<size_t>(argco.val_number);
        TValue *cfunc = __get_register(V, rfn.val_register);

        // Call function
        __extern_call(V, cfunc->val_cfunction, argc);
        VM_NEXT();
    }
    case OpCode::NATIVECALL: {
        Operand rfn = V->ip->operand1;
        Operand argco = V->ip->operand2;
        size_t argc = static_cast<size_t>(argco.val_number);
        TValue *func = __get_register(V, rfn.val_register);

        // Call function
        __native_call(V, func->val_function, argc);
        VM_NEXT();
    }
    case OpCode::METHODCALL: {
        Operand robj = V->ip->operand1;
        Operand rfn = V->ip->operand2;
        Operand argco = V->ip->operand3;

        size_t argc = static_cast<size_t>(argco.val_number);
        TValue *func = __get_register(V, rfn.val_register);
        TValue *obj = __get_register(V, robj.val_register);

        __push(V, *obj); // Push self
        // Call function, with [argc + 1] to account for the self argument
        __native_call(V, func->val_function, argc + 1);
        VM_NEXT();
    }

    case OpCode::RETURN: {
        Operand retcv = V->ip->operand1;
        size_t retc = static_cast<size_t>(retcv.val_number);

        __native_ret(V, retc);
        VM_NEXT();
    }

    case OpCode::GETTABLE: {
        Operand rdst = V->ip->operand1;
        Operand rtbl = V->ip->operand2;
        Operand ridx = V->ip->operand3;

        TValue &tbl = *__get_register(V, rtbl.val_register);
        TValue &idx = *__get_register(V, ridx.val_register);

        // Get table key based on the index type (string or number)
        TableKey key = check_string(idx) ? idx.val_string->hash : idx.val_number;
        const TValue &index = __get_table(tbl.val_table, key, true);

        __set_register(V, rdst.val_register, index);
        VM_NEXT();
    }

    case OpCode::SETTABLE: {
        Operand rsrc = V->ip->operand1;
        Operand rtbl = V->ip->operand2;
        Operand ridx = V->ip->operand3;

        TValue val;
        TValue &tbl = *__get_register(V, rtbl.val_register);
        TValue &idx = *__get_register(V, ridx.val_register);

        // Get table key based on the index type (string or number)
        TableKey key = check_string(idx) ? idx.val_string->hash : static_cast<Hash>(idx.val_number);
        // Slow-path: the value is stored in a register, load it
        if (VIA_UNLIKELY(rsrc.type == OperandType::Register)) {
            TValue &temp = *__get_register(V, rsrc.val_register);
            val = std::move(temp);
        }
        else {
            val = TValue(rsrc);
        }

        // Set the table index
        TTable *ltbl = tbl.val_table;
        __set_table(ltbl, key, val);
        VM_NEXT();
    }

    case OpCode::NEXTTABLE: {
        static std::unordered_map<void *, TableKey> next_table;

        Operand dst = V->ip->operand1;
        Operand valr = V->ip->operand2;

        TValue *val = __get_register(V, valr.val_register);
        void *ptr = __to_pointer(*val);
        TableKey key = 0;

        // Look for the current key in next_table and increment it if found
        auto it = next_table.find(ptr);
        if (it != next_table.end()) {
            key = ++it->second;
        }
        else {
            next_table[ptr] = 0;
        }

        const auto &table_map = val->val_table->data;
        auto field_it = table_map.find(key);

        if (field_it != table_map.end()) {
            // If the key is found, use the corresponding value
            __set_register(V, dst.val_register, field_it->second);
        }
        else { // If not found, set the value to a default (e.g., _Nil)
            __set_register(V, dst.val_register, _Nil);
        }

        VM_NEXT();
    }

    case OpCode::LENTABLE: {
        Operand dst = V->ip->operand1;
        Operand tblr = V->ip->operand2;

        TValue *val = __get_register(V, tblr.val_register);
        TNumber size = static_cast<TNumber>(val->val_table->data.size());
        TValue val_size(size);

        __set_register(V, dst.val_register, val_size);
        VM_NEXT();
    }

    case OpCode::LENSTRING: {
        Operand rdst = V->ip->operand1;
        Operand objr = V->ip->operand2;

        TValue *val = __get_register(V, objr.val_register);
        TNumber len = static_cast<TNumber>(val->val_string->len);
        TValue val_len(len);

        __set_register(V, rdst.val_register, val_len);
        VM_NEXT();
    }

    case OpCode::GETSTRING: {
        Operand dst = V->ip->operand1;
        Operand str = V->ip->operand2;
        Operand idx = V->ip->operand3;

        TValue *str_val = __get_register(V, str.val_register);
        TValue *idx_val = __get_register(V, idx.val_register);

        size_t index = idx_val->val_number;
        if (VIA_UNLIKELY(index > str_val->val_string->len)) {
            __set_register(V, dst.val_register, _Nil);
        }

        VM_NEXT();
    }

    case OpCode::LEN: {
        Operand rdst = V->ip->operand1;
        Operand objr = V->ip->operand2;

        TValue *val = __get_register(V, objr.val_register);
        TValue len = __len(V, *val);

        __set_register(V, rdst.val_register, len);
        VM_NEXT();
    }

    case OpCode::TYPE: {
        Operand rdst = V->ip->operand1;
        Operand objr = V->ip->operand2;

        TValue *val = __get_register(V, objr.val_register);
        TValue ty = __type(V, *val);

        __set_register(V, rdst.val_register, ty);
        VM_NEXT();
    }

    case OpCode::TYPEOF: {
        Operand rdst = V->ip->operand1;
        Operand objr = V->ip->operand2;

        TValue *val = __get_register(V, objr.val_register);
        TValue type = __typeofv(V, *val);

        __set_register(V, rdst.val_register, type);
        VM_NEXT();
    }

    default:
        VM_FATAL(std::format("unknown opcode {:x}", static_cast<int>(V->ip->op)));
    }
}

exit:
    V->sig_exit.fire();
}

// Permanently kills the thread. Does not clean up the state object.
void kill_thread(State *VIA_RESTRICT V)
{
    if (V->tstate == ThreadState::RUNNING) {
        V->abort = true;
        V->sig_exit.wait();
    }

    // Mark as dead thread
    V->tstate = ThreadState::DEAD;
    // Decrement the thread_id to make room for more threads (I know you can techni__cally make 2^32 threads ok?)
    V->G->threads.fetch_add(-1);
}

// Temporarily pauses the thread.
void pausethread(State *VIA_RESTRICT V)
{
    if (V->tstate == ThreadState::RUNNING) {
        V->abort = true;
        V->sig_exit.wait();
    }

    V->tstate = ThreadState::PAUSED;
}

} // namespace via
