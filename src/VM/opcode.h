/* This file is a part of the via programming language at
 * https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

namespace via
{

/*
 * OpCode operand convention
 * <opcode> <registers> <identifiers> <everything-else>
 * Ordered from most likely to be executed to the least. (Except NOP)
 */
enum class OpCode
{
    /*
     * opcode: NOP
     * No operation. Can invoke empty instruction jump optimizations.
     * operand0: <>
     * operand1: <>
     * operand2: <>
     */
    NOP,
    /*
     * opcode: ADD
     * Adds the value in operand1 to operand0.
     * operand0: <dst :: Register>
     * operand1: <src :: Register>
     * operand2: <>
     */
    ADD,
    /*
     * opcode: ADDK
     * Adds the constant value from the table at operand1 to operand0.
     * operand0: <dst :: Register>
     * operand1: <idx :: Number>
     * operand2: <>
     */
    ADDK,
    /*
     * opcode: ADDI
     * Adds the immediate value in operand1 to operand0.
     * operand0: <dst :: Register>
     * operand1: <imm :: Number>
     * operand2: <>
     */
    ADDI,
    /*
     * opcode: SUB
     * Subtracts the value in operand1 from operand0.
     * operand0: <dst :: Register>
     * operand1: <src :: Register>
     * operand2: <>
     */
    SUB,
    /*
     * opcode: SUBK
     * Subtracts the constant value from the table at operand1 from operand0.
     * operand0: <dst :: Register>
     * operand1: <idx :: Number>
     * operand2: <>
     */
    SUBK,
    /*
     * opcode: SUBI
     * Subtracts the immediate value in operand1 from operand0.
     * operand0: <dst :: Register>
     * operand1: <imm :: Number>
     * operand2: <>
     */
    SUBI,
    /*
     * opcode: MUL
     * Multiplies operand0 by the value in operand1.
     * operand0: <dst :: Register>
     * operand1: <src :: Register>
     * operand2: <>
     */
    MUL,
    /*
     * opcode: MULK
     * Multiplies operand0 by the constant value from the table at operand1.
     * operand0: <dst :: Register>
     * operand1: <idx :: Number>
     * operand2: <>
     */
    MULK,
    /*
     * opcode: MULI
     * Multiplies operand0 by the immediate value in operand1.
     * operand0: <dst :: Register>
     * operand1: <imm :: Number>
     * operand2: <>
     */
    MULI,
    /*
     * opcode: DIV
     * Divides operand0 by the value in operand1.
     * operand0: <dst :: Register>
     * operand1: <src :: Register>
     * operand2: <>
     */
    DIV,
    /*
     * opcode: DIVK
     * Divides operand0 by the constant value from the table at operand1.
     * operand0: <dst :: Register>
     * operand1: <idx :: Number>
     * operand2: <>
     */
    DIVK,
    /*
     * opcode: DIVI
     * Divides operand0 by the immediate value in operand1.
     * operand0: <dst :: Register>
     * operand1: <imm :: Number>
     * operand2: <>
     */
    DIVI,
    /*
     * opcode: MOD
     * Computes the remainder of operand0 divided by the value in operand1.
     * operand0: <dst :: Register>
     * operand1: <src :: Register>
     * operand2: <>
     */
    MOD,
    /*
     * opcode: MODK
     * Computes the remainder of operand0 divided by the constant value from the table at operand1.
     * operand0: <dst :: Register>
     * operand1: <idx :: Number>
     * operand2: <>
     */
    MODK,
    /*
     * opcode: MODI
     * Computes the remainder of operand0 divided by the immediate value in operand1.
     * operand0: <dst :: Register>
     * operand1: <imm :: Number>
     * operand2: <>
     */
    MODI,
    /*
     * opcode: POW
     * Raises operand0 to the power of the value in operand1.
     * operand0: <dst :: Register>
     * operand1: <src :: Register>
     * operand2: <>
     */
    POW,
    /*
     * opcode: POWK
     * Raises operand0 to the power of the constant value from the table at operand1.
     * operand0: <dst :: Register>
     * operand1: <idx :: Number>
     * operand2: <>
     */
    POWK,
    /*
     * opcode: POWI
     * Raises operand0 to the power of the immediate value in operand1.
     * operand0: <dst :: Register>
     * operand1: <imm :: Number>
     * operand2: <>
     */
    POWI,
    /*
     * opcode: NEG
     * Negates the value in operand0.
     * operand0: <val :: Register>
     * operand1: <>
     * operand2: <>
     */
    NEG,
    /*
     * opcode: NEGK
     * Negates the constant at idx operand1 and loads the result into operand0.
     * operand0: <dst :: Register>
     * operand1: <idx :: Number>
     * operand2: <>
     */
    NEGK,
    /*
     * opcode: MOVE
     * Moves the value in operand0 to operand1.
     * operand0: <dst :: Register>
     * operand1: <src :: Register>
     * operand2: <>
     */
    MOVE,
    /*
     * opcode: SWAP
     * Swaps the values of operand0 and operand1.
     * operand0: <r0 :: Register>
     * operand1: <r1 :: Register>
     * operand2: <>
     */
    SWAP,
    /*
     * opcode: LOADK
     * Loads constant from the constant table at idx operand1 into operand0.
     * operand0: <dst :: Register>
     * operand1: <idx :: Number>
     * operand2: <>
     */
    LOADK,
    /*
     * opcode: LOADNIL
     * Loads immediate nil into dst.
     * operand0: <dst :: Register>
     * operand1: <>
     * operand2: <>
     */
    LOADNIL,
    /*
     * opcode: LOADNUMBER
     * Loads immediate number operand1 into dst.
     * operand0: <dst :: Register>
     * operand1: <num :: Number>
     * operand2: <>
     */
    LOADNUMBER,
    /*
     * opcode: LOADBOOL
     * Loads immediate bool operand1 into dst.
     * operand0: <dst :: Register>
     * operand1: <num :: Boolean>
     * operand2: <>
     */
    LOADBOOL,
    /*
     * opcode: LOADSTRING
     * Loads immediate string operand1 into dst.
     * operand0: <dst :: Register>
     * operand1: <str :: String>
     * operand2: <>
     */
    LOADSTRING,
    /*
     * opcode: LOADTABLE
     * Loads empty immediate table into dst.
     * operand0: <dst :: Register>
     * operand1: <>
     * operand2: <>
     */
    LOADTABLE,
    /*
     * opcode: LOADFUNCTION
     * Loads immediate function into dst, captures all bytecode from itself to the next RETURN instruction.
     * operand0: <dst :: Register>
     * operand1: <>
     * operand2: <>
     */
    LOADFUNCTION,
    /*
     * opcode: PUSH
     * Pushes a value onto the stack.
     * operand0: <src :: Register>
     * operand1: <>
     * operand2: <>
     */
    PUSH,
    PUSHK,
    PUSHI,
    /*
     * opcode: POP
     * Pops and loads the top-most element of the stack onto a register.
     * operand0: <dst :: Register>
     * operand1: <>
     * operand2: <>
     */
    POP,
    /*
     * opcode: GETSTACK
     * Loads a local variable relative to the sbp (stack base pointer) into register <dst> with offset <off>.
     * operand0: <dst :: Register>
     * operand1: <off :: Number>
     * operand2: <>
     */
    GETSTACK,
    /*
     * opcode: SETSTACK
     * Sets a local variable relative to the sbp (stack base pointer) to the value in <src> with offset <off>.
     * operand0: <src :: Register>
     * operand1: <off :: Number>
     * operand2: <>
     */
    SETSTACK,
    /*
     * opcode: GETARGUMENT
     * Loads the argument at stack offset [ssp + argc - 1 - off] into register <dst>.
     * operand0: <dst :: Register>
     * operand1: <off :: Number>
     * operand2: <>
     */
    GETARGUMENT,
    /*
     * opcode: GETGLOBAL
     * Loads the global with id <id> into <dst>, nil if impossible.
     * operand0: <dst :: Register>
     * operand1: <id :: Identifier>
     * operand2: <>
     */
    GETGLOBAL,
    /*
     * opcode: SETGLOBAL
     * Attempts to declare a new global constant.
     * operand0: <val :: Register>
     * operand1: <id :: Identifier>
     * operand2: <>
     */
    SETGLOBAL,
    /*
     * opcode: INCREMENT
     * Increments the value in operand0.
     * operand0: <dst :: Register>
     * operand1: <>
     * operand2: <>
     */
    INCREMENT,
    /*
     * opcode: DECREMENT
     * Decrements the value in operand0.
     * operand0: <dst :: Register>
     * operand1: <>
     * operand2: <>
     */
    DECREMENT,
    LOADARGUMENT,
    /*
     * opcode: EQUAL
     * Performs a comparison operation between operand1 and operand2. Stores the
     * result in operand0.
     * operand0: <src :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    EQUAL,
    /*
     * opcode: NOTEQUAL
     * Performs a reversed comparison operation between operand1 and operand2.
     * Stores the result in operand0. operand0: <src :: Register> operand1: <lhs
     * :: Register> operand2: <rhs :: Register>
     */
    NOTEQUAL,
    /*
     * opcode: LESS
     * Performs a less-than comparison operation between operand1 and operand2.
     * Stores the result in operand0. operand0: <src :: Register> operand1: <lhs
     * :: Register> operand2: <rhs :: Register>
     */
    LESS,
    /*
     * opcode: GREATER
     * Performs a greater-than comparison operation between operand1 and operand2.
     * Stores the result in operand0. operand0: <src :: Register> operand1: <lhs
     * :: Register> operand2: <rhs :: Register>
     */
    GREATER,
    /*
     * opcode: LESSOREQUAL
     * Performs a less-than-or-equal-to comparison operation between operand1 and
     * operand2. Stores the result in operand0. operand0: <src :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    LESSOREQUAL,
    /*
     * opcode: GREATEROREQUAL
     * Performs a greater-than-or-equal-to comparison operation between operand1
     * and operand2. Stores the result in operand0. operand0: <src :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    GREATEROREQUAL,
    /*
     * opcode: JUMP
     * Unconditinally jumps given offset in operand0. Does not save the state.
     * operand0: <off :: Number>
     * operand1: <>
     * operand2: <>
     */
    JUMP,
    /*
     * opcode: JUMPIFNOT
     * Jumps given offset in operand0 if the value in operand1 is not equal to 0.
     * Does not save the state. operand0: <off :: Number> operand1: <cnd ::
     * Register> operand2: <>
     */
    JUMPIFNOT,
    /*
     * opcode: JUMPIF
     * Jumps given offset in operand0 if the value in operand1 is equal to 0. Does
     * not save the state. operand0: <off :: Number> operand1: <cnd :: Register>
     * operand2: <>
     */
    JUMPIF,
    /*
     * opcode: JUMPIFEQUAL
     * Jumps given offset in operand0 if the value in operand1 is equal to the
     * value in operand2. Does not save the state. operand0: <off :: Number>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    JUMPIFEQUAL,
    /*
     * opcode: JUMPIFNOTEQUAL
     * Jumps given offset in operand0 if the value in operand1 is not equal to the
     * value in operand2. Does not save the state. operand0: <off :: Number>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    JUMPIFNOTEQUAL,
    /*
     * opcode: JUMPIFLESS
     * Jumps given offset in operand0 if the value in operand1 is less than the
     * value in operand2. Does not save the state. operand0: <off :: Number>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    JUMPIFLESS,
    /*
     * opcode: JUMPIFGREATER
     * Jumps given offset in operand0 if the value in operand1 greater than the
     * value in operand2. Does not save the state. operand0: <off :: Number>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    JUMPIFGREATER,
    /*
     * opcode: JUMPIFLESSOREQUAL
     * Jumps given offset in operand0 if the value in operand1 is less than or
     * equal to the value in operand2. Does not save the state. operand0: <off ::
     * Number> operand1: <lhs :: Register> operand2: <rhs :: Register>
     */
    JUMPIFLESSOREQUAL,
    /*
     * opcode: JUMPIFGREATEROREQUAL
     * Jumps given offset in operand0 if the value in operand1 is greater than or
     * equal to the value in operand2. Does not save the state. operand0: <off ::
     * Number> operand1: <lhs :: Register> operand2: <rhs :: Register>
     */
    JUMPIFGREATEROREQUAL,
    /*
     * opcode: CALL
     * Calls the value in operand0 with arg count in operand1. Works for all callable types.
     * operand0: <callee :: Register>
     * operand1: <argc :: Number>
     * operand2: <>
     */
    CALL,
    /*
     * opcode: EXTERNCALL
     * Calls the value in operand0 with arg count in operand1. Only works for C functions.
     * operand0: <callee :: Register>
     * operand1: <argc :: Number>
     * operand2: <>
     */
    EXTERNCALL,
    /*
     * opcode: NATIVECALL
     * Calls the value in operand0 with arg count in operand1. Only works for native functions.
     * operand0: <callee :: Register>
     * operand1: <argc :: Number>
     * operand2: <>
     */
    NATIVECALL,
    /*
     * opcode: METHODCALL
     * Calls the value in operand0 with arg count in operand1, loads object as self (arg0). Only works for native functions.
     * operand0: <object :: Register>
     * operand1: <method :: Register>
     * operand2: <argc :: Number>
     */
    METHODCALL,
    /*
     * opcode: RETURN
     * Performs a return operation by jumping back to the return address of the
     * current stack, and popping it. Only works inside function scopes. operand0:
     * <> operand1: <> operand2: <>
     */
    RETURN,
    /*
     * opcode: EXIT
     * Performs an exit with the exit code in operand0 and an automatically
     * deduced exit message. operand0: <code :: Number> operand1: <> operand2: <>
     */
    EXIT,
    /*
     * opcode: GETTABLE
     * Loads the index operand2 of table operand1 into operand0.
     * operand0: <dst :: Register>
     * operand1: <tbl :: Register>
     * operand2: <idx :: Register>
     */
    GETTABLE,
    /*
     * opcode: SETTABLE
     * Sets the index operand1 of table operand0 to value in operand2.
     * operand0: <tbl :: Register>
     * operand1: <idx :: Register>
     * operand2: <val :: Register>
     */
    SETTABLE,
    /*
     * opcode: NEXTTABLE
     * Puts the next value inside table operand1 relative to the last invocation.
     * Used for iteration.
     * operand0: <dst :: Register>
     * operand1: <tbl :: Register>
     * operand2: <>
     */
    NEXTTABLE,
    /*
     * opcode: LENTABLE
     * Stores the length of the table in operand1 in operand0.
     * operand0: <dst :: Register>
     * operand1: <val :: Register>
     * operand2: <>
     */
    LENTABLE,
    /*
     * opcode: CONCAT
     * Concatenates operand0 with operand1 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <val :: Register>
     * operand2: <>
     */
    CONCAT,
    /*
     * opcode: CONCATK
     * Concatenates operand0 with constant at index operand1 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <val :: Register>
     * operand2: <>
     */
    CONCATK,
    /*
     * opcode: CONCATI
     * Concatenates operand0 with immedate operand1 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <imm :: String>
     * operand2: <>
     */
    CONCATI,
    /*
     * opcode: SETSTRING
     * Numberes into string in operand1 with operand2 and loads the value into operand0.
     * operand0: <dst :: Register>
     * operand1: <str :: Register>
     * operand2: <idx :: Register>
     */
    GETSTRING,
    /*
     * opcode: SETSTRING
     * Sets the index in operand1 of string stored in operand0 to value in operand2.
     * operand0: <str :: Register>
     * operand1: <idx :: Register>
     * operand2: <val :: Register>
     */
    SETSTRING,
    /*
     * opcode: LENSTRING
     * Stores the length of the string in operand1 in operand0.
     * operand0: <dst :: Register>
     * operand1: <val :: Register>
     * operand2: <>
     */
    LENSTRING,
    /*
     * opcode: LEN
     * Stores the length of the object in operand1 in operand0.
     * operand0: <dst :: Register>
     * operand1: <val :: Register>
     * operand2: <>
     */
    LEN,
    /*
     * opcode: TYPEOF
     * Stores the non-primitive type of operand1 in operand0.
     * operand0: <dst :: Register>
     * operand1: <val :: Register>
     * operand2: <>
     */
    TYPEOF,
    /*
     * opcode: TYPE
     * Stores the primitive type of operand1 in operand0.
     * operand0: <dst :: Register>
     * operand1: <val :: Register>
     * operand2: <>
     */
    TYPE,
};

} // namespace via
