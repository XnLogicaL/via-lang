// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

namespace via {

/*
 * OpCode operand convention
 * <opcode> <registers> <identifiers> <everything-else>
 * Ordered from most likely to be executed to the least. (Except NOP)
 */
enum class OpCode {
    /*
     * opcode: NOP
     * No operation. Can invoke empty instruction jump optimizations.
     * operand0: <>
     * operand0: <>
     * operand1: <>
     */
    NOP,
    /*
     * opcode: ADD
     * Adds the value in operand0 to operand0.
     * operand0: <dst :: Register>
     * operand0: <src :: Register>
     * operand1: <>
     */
    ADD,
    /*
     * opcode: ADDK
     * Adds the constant value from the table at operand0 to operand0.
     * operand0: <dst :: Register>
     * operand0: <idx :: Number>
     * operand1: <>
     */
    ADDK,
    /*
     * opcode: SUB
     * Subtracts the value in operand0 from operand0.
     * operand0: <dst :: Register>
     * operand0: <src :: Register>
     * operand1: <>
     */
    SUB,
    /*
     * opcode: SUBK
     * Subtracts the constant value from the table at operand0 from operand0.
     * operand0: <dst :: Register>
     * operand0: <idx :: Number>
     * operand1: <>
     */
    SUBK,
    /*
     * opcode: MUL
     * Multiplies operand0 by the value in operand0.
     * operand0: <dst :: Register>
     * operand0: <src :: Register>
     * operand1: <>
     */
    MUL,
    /*
     * opcode: MULK
     * Multiplies operand0 by the constant value from the table at operand0.
     * operand0: <dst :: Register>
     * operand0: <idx :: Number>
     * operand1: <>
     */
    MULK,
    /*
     * opcode: DIV
     * Divides operand0 by the value in operand0.
     * operand0: <dst :: Register>
     * operand0: <src :: Register>
     * operand1: <>
     */
    DIV,
    /*
     * opcode: DIVK
     * Divides operand0 by the constant value from the table at operand0.
     * operand0: <dst :: Register>
     * operand0: <idx :: Number>
     * operand1: <>
     */
    DIVK,
    /*
     * opcode: MOD
     * Computes the remainder of operand0 divided by the value in operand0.
     * operand0: <dst :: Register>
     * operand0: <src :: Register>
     * operand1: <>
     */
    MOD,
    /*
     * opcode: MODK
     * Computes the remainder of operand0 divided by the constant value from the table at operand0.
     * operand0: <dst :: Register>
     * operand0: <idx :: Number>
     * operand1: <>
     */
    MODK,
    /*
     * opcode: POW
     * Raises operand0 to the power of the value in operand0.
     * operand0: <dst :: Register>
     * operand0: <src :: Register>
     * operand1: <>
     */
    POW,
    /*
     * opcode: POWK
     * Raises operand0 to the power of the constant value from the table at operand0.
     * operand0: <dst :: Register>
     * operand0: <idx :: Number>
     * operand1: <>
     */
    POWK,
    /*
     * opcode: NEG
     * Negates the value in operand0.
     * operand0: <val :: Register>
     * operand0: <>
     * operand1: <>
     */
    NEG,
    /*
     * opcode: NEGK
     * Negates the constant at idx operand0 and loads the result into operand0.
     * operand0: <dst :: Register>
     * operand0: <idx :: Number>
     * operand1: <>
     */
    NEGK,
    /*
     * opcode: MOVE
     * Moves the value in operand0 to operand0.
     * operand0: <dst :: Register>
     * operand0: <src :: Register>
     * operand1: <>
     */
    MOVE,
    /*
     * opcode: SWAP
     * Swaps the values of operand0 and operand0.
     * operand0: <r0 :: Register>
     * operand0: <r1 :: Register>
     * operand1: <>
     */
    SWAP,
    /*
     * opcode: LOADK
     * Loads constant from the constant table at idx operand0 into operand0.
     * operand0: <dst :: Register>
     * operand0: <idx :: Number>
     * operand1: <>
     */
    LOADK,
    /*
     * opcode: LOADNIL
     * Loads immediate nil into dst.
     * operand0: <dst :: Register>
     * operand0: <>
     * operand1: <>
     */
    LOADNIL,
    /*
     * opcode: LOADTABLE
     * Loads empty immediate table into dst.
     * operand0: <dst :: Register>
     * operand0: <>
     * operand1: <>
     */
    LOADTABLE,
    /*
     * opcode: LOADFUNCTION
     * Loads immediate function into dst, captures all bytecode from itself to the next RETURN instruction.
     * operand0: <dst :: Register>
     * operand0: <>
     * operand1: <>
     */
    LOADFUNCTION,
    /*
     * opcode: PUSH
     * Pushes a value onto the stack.
     * operand0: <src :: Register>
     * operand0: <>
     * operand1: <>
     */
    PUSH,
    /*
     * opcode: PUSHK
     * Pushes a constant value onto the stack.
     * operand0: <const_idx :: Number>
     * operand0: <>
     * operand1: <>
     */
    PUSHK,
    /*
     * opcode: PUSHI
     * Pushes an immediate value onto the stack.
     * operand0: <imm :: Any>
     * operand0: <>
     * operand1: <>
     */
    PUSHI,
    /*
     * opcode: POP
     * Pops and loads the top-most element of the stack onto a register.
     * operand0: <dst :: Register>
     * operand0: <>
     * operand1: <>
     */
    POP,
    /*
     * opcode: GETSTACK
     * Loads a local variable relative to the sbp (stack base pointer) into register <dst> with offset <off>.
     * operand0: <dst :: Register>
     * operand0: <off :: Number>
     * operand1: <>
     */
    GETSTACK,
    /*
     * opcode: SETSTACK
     * Sets a local variable relative to the sbp (stack base pointer) to the value in <src> with offset <off>.
     * operand0: <src :: Register>
     * operand0: <off :: Number>
     * operand1: <>
     */
    SETSTACK,
    /*
     * opcode: GETARGUMENT
     * Loads the argument at stack offset [ssp + argc - 1 - off] into register <dst>.
     * operand0: <dst :: Register>
     * operand0: <off :: Number>
     * operand1: <>
     */
    GETARGUMENT,
    /*
     * opcode: GETGLOBAL
     * Loads the global with id <id> into <dst>, nil if impossible.
     * operand0: <dst :: Register>
     * operand0: <id :: Identifier>
     * operand1: <>
     */
    GETGLOBAL,
    /*
     * opcode: SETGLOBAL
     * Attempts to declare a new global constant.
     * operand0: <val :: Register>
     * operand0: <id :: Identifier>
     * operand1: <>
     */
    SETGLOBAL,
    /*
     * opcode: INCREMENT
     * Increments the value in operand0.
     * operand0: <dst :: Register>
     * operand0: <>
     * operand1: <>
     */
    INCREMENT,
    /*
     * opcode: DECREMENT
     * Decrements the value in operand0.
     * operand0: <dst :: Register>
     * operand0: <>
     * operand1: <>
     */
    DECREMENT,
    /*
     * opcode: EQUAL
     * Performs a comparison operation between operand0 and operand1. Stores the
     * result in operand0.
     * operand0: <src :: Register>
     * operand0: <lhs :: Register>
     * operand1: <rhs :: Register>
     */
    EQUAL,
    /*
     * opcode: NOTEQUAL
     * Performs a reversed comparison operation between operand0 and operand1.
     * Stores the result in operand0. operand0: <src :: Register> operand0: <lhs
     * :: Register> operand1: <rhs :: Register>
     */
    NOTEQUAL,
    /*
     * opcode: LESS
     * Performs a less-than comparison operation between operand0 and operand1.
     * Stores the result in operand0. operand0: <src :: Register> operand0: <lhs
     * :: Register> operand1: <rhs :: Register>
     */
    LESS,
    /*
     * opcode: GREATER
     * Performs a greater-than comparison operation between operand0 and operand1.
     * Stores the result in operand0. operand0: <src :: Register> operand0: <lhs
     * :: Register> operand1: <rhs :: Register>
     */
    GREATER,
    /*
     * opcode: LESSOREQUAL
     * Performs a less-than-or-equal-to comparison operation between operand0 and
     * operand1. Stores the result in operand0. operand0: <src :: Register>
     * operand0: <lhs :: Register>
     * operand1: <rhs :: Register>
     */
    LESSOREQUAL,
    /*
     * opcode: GREATEROREQUAL
     * Performs a greater-than-or-equal-to comparison operation between operand0
     * and operand1. Stores the result in operand0. operand0: <src :: Register>
     * operand0: <lhs :: Register>
     * operand1: <rhs :: Register>
     */
    GREATEROREQUAL,
    /*
     * opcode: JUMP
     * Unconditinally jumps given offset in operand0. Does not save the state.
     * operand0: <off :: Number>
     * operand0: <>
     * operand1: <>
     */
    JUMP,
    /*
     * opcode: JUMPIFNOT
     * Jumps given offset in operand0 if the value in operand0 is not equal to 0.
     * Does not save the state. operand0: <off :: Number> operand0: <cnd ::
     * Register> operand1: <>
     */
    JUMPIFNOT,
    /*
     * opcode: JUMPIF
     * Jumps given offset in operand0 if the value in operand0 is equal to 0. Does
     * not save the state. operand0: <off :: Number> operand0: <cnd :: Register>
     * operand1: <>
     */
    JUMPIF,
    /*
     * opcode: JUMPIFEQUAL
     * Jumps given offset in operand0 if the value in operand0 is equal to the
     * value in operand1. Does not save the state. operand0: <off :: Number>
     * operand0: <lhs :: Register>
     * operand1: <rhs :: Register>
     */
    JUMPIFEQUAL,
    /*
     * opcode: JUMPIFNOTEQUAL
     * Jumps given offset in operand0 if the value in operand0 is not equal to the
     * value in operand1. Does not save the state. operand0: <off :: Number>
     * operand0: <lhs :: Register>
     * operand1: <rhs :: Register>
     */
    JUMPIFNOTEQUAL,
    /*
     * opcode: JUMPIFLESS
     * Jumps given offset in operand0 if the value in operand0 is less than the
     * value in operand1. Does not save the state. operand0: <off :: Number>
     * operand0: <lhs :: Register>
     * operand1: <rhs :: Register>
     */
    JUMPIFLESS,
    /*
     * opcode: JUMPIFGREATER
     * Jumps given offset in operand0 if the value in operand0 greater than the
     * value in operand1. Does not save the state. operand0: <off :: Number>
     * operand0: <lhs :: Register>
     * operand1: <rhs :: Register>
     */
    JUMPIFGREATER,
    /*
     * opcode: JUMPIFLESSOREQUAL
     * Jumps given offset in operand0 if the value in operand0 is less than or
     * equal to the value in operand1. Does not save the state. operand0: <off ::
     * Number> operand0: <lhs :: Register> operand1: <rhs :: Register>
     */
    JUMPIFLESSOREQUAL,
    /*
     * opcode: JUMPIFGREATEROREQUAL
     * Jumps given offset in operand0 if the value in operand0 is greater than or
     * equal to the value in operand1. Does not save the state. operand0: <off ::
     * Number> operand0: <lhs :: Register> operand1: <rhs :: Register>
     */
    JUMPIFGREATEROREQUAL,
    /*
     * opcode: CALL
     * Calls the value in operand0 with arg count in operand0. Works for all callable types.
     * operand0: <callee :: Register>
     * operand0: <argc :: Number>
     * operand1: <>
     */
    CALL,
    /*
     * opcode: EXTERNCALL
     * Calls the value in operand0 with arg count in operand0. Only works for C functions.
     * operand0: <callee :: Register>
     * operand0: <argc :: Number>
     * operand1: <>
     */
    EXTERNCALL,
    /*
     * opcode: NATIVECALL
     * Calls the value in operand0 with arg count in operand0. Only works for native functions.
     * operand0: <callee :: Register>
     * operand0: <argc :: Number>
     * operand1: <>
     */
    NATIVECALL,
    /*
     * opcode: METHODCALL
     * Calls the value in operand0 with arg count in operand0, loads object as self (arg0). Only works for native functions.
     * operand0: <object :: Register>
     * operand0: <method :: Register>
     * operand1: <argc :: Number>
     */
    METHODCALL,
    /*
     * opcode: RETURN
     * Performs a return operation by jumping back to the return address of the
     * current stack, and popping it. Only works inside function scopes. operand0:
     * <> operand0: <> operand1: <>
     */
    RETURN,
    /*
     * opcode: EXIT
     * Performs an exit with the exit code in operand0 and an automatically
     * deduced exit message. operand0: <code :: Number> operand0: <> operand1: <>
     */
    EXIT,
    /*
     * opcode: GETTABLE
     * Loads the index operand1 of table operand0 into operand0.
     * operand0: <dst :: Register>
     * operand0: <tbl :: Register>
     * operand1: <idx :: Register>
     */
    GETTABLE,
    /*
     * opcode: SETTABLE
     * Sets the index operand0 of table operand0 to value in operand1.
     * operand0: <tbl :: Register>
     * operand0: <idx :: Register>
     * operand1: <val :: Register>
     */
    SETTABLE,
    /*
     * opcode: NEXTTABLE
     * Puts the next value inside table operand0 relative to the last invocation.
     * Used for iteration.
     * operand0: <dst :: Register>
     * operand0: <tbl :: Register>
     * operand1: <>
     */
    NEXTTABLE,
    /*
     * opcode: LENTABLE
     * Stores the length of the table in operand0 in operand0.
     * operand0: <dst :: Register>
     * operand0: <val :: Register>
     * operand1: <>
     */
    LENTABLE,
    /*
     * opcode: CONCAT
     * Concatenates operand0 with operand0 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand0: <val :: Register>
     * operand1: <>
     */
    CONCAT,
    /*
     * opcode: CONCATK
     * Concatenates operand0 with constant at index operand0 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand0: <val :: Register>
     * operand1: <>
     */
    CONCATK,
    /*
     * opcode: CONCATI
     * Concatenates operand0 with immedate operand0 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand0: <imm :: String>
     * operand1: <>
     */
    CONCATI,
    /*
     * opcode: SETSTRING
     * Numberes into string in operand0 with operand1 and loads the value into operand0.
     * operand0: <dst :: Register>
     * operand0: <str :: Register>
     * operand1: <idx :: Register>
     */
    GETSTRING,
    /*
     * opcode: SETSTRING
     * Sets the index in operand0 of string stored in operand0 to value in operand1.
     * operand0: <str :: Register>
     * operand0: <idx :: Register>
     * operand1: <val :: Register>
     */
    SETSTRING,
    /*
     * opcode: LENSTRING
     * Stores the length of the string in operand0 in operand0.
     * operand0: <dst :: Register>
     * operand0: <val :: Register>
     * operand1: <>
     */
    LENSTRING,
    /*
     * opcode: LEN
     * Stores the length of the object in operand0 in operand0.
     * operand0: <dst :: Register>
     * operand0: <val :: Register>
     * operand1: <>
     */
    LEN,
    /*
     * opcode: TYPEOF
     * Stores the non-primitive type of operand0 in operand0.
     * operand0: <dst :: Register>
     * operand0: <val :: Register>
     * operand1: <>
     */
    TYPEOF,
    /*
     * opcode: TYPE
     * Stores the primitive type of operand0 in operand0.
     * operand0: <dst :: Register>
     * operand0: <val :: Register>
     * operand1: <>
     */
    TYPE,
};

} // namespace via
