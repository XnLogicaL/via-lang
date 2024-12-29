/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

namespace via
{

/*
 * OpCode operand convention
 * <opcode> <registers> <identifiers> <everything-else>
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
     * opcode: MOV
     * Moves the value in operand0 to operand1. Cleans up operand0 after the operation.
     * operand0: <dst :: Register>
     * operand1: <src :: Register>
     * operand2: <>
     */
    MOV,
    /*
     * opcode: CPY
     * Copies the value in operand0 into operand1 without modifying operand0.
     * operand0: <dst :: Register>
     * operand1: <src :: Register>
     * operand2: <>
     */
    CPY,
    /*
     * opcode: LOAD
     * Loads immediate value operand1 into operand0.
     * operand0: <dst :: Register>
     * operand1: <val :: Any>
     * operand2: <>
     */
    LOAD,
    /*
     * opcode: SWAP
     * Swaps the values of operand0 and operand1.
     * operand0: <r0 :: Register>
     * operand1: <r1 :: Register>
     * operand2: <>
     */
    SWAP,
    /*
     * opcode: PUSH
     * Pushes an anonymous stack frame onto the stack.
     * operand0: <>
     * operand1: <>
     * operand2: <>
     */
    PUSH,
    /*
     * opcode: POP
     * Pops the top-most stack frame on to stack.
     * operand0: <>
     * operand1: <>
     * operand2: <>
     */
    POP,
    /*
     * opcode: SETLOCAL
     * Sets a local variable with the value of operand0 and identifier of operand1.
     * operand0: <src :: Register>
     * operand1: <id :: Identifier>
     * operand2: <>
     */
    SETLOCAL,
    /*
     * opcode: LOADLOCAL
     * Loads a local variable with identifier operand1 into operand0.
     * operand0: <dst :: Register>
     * operand1: <id :: Identifier>
     * operand2: <>
     */
    LOADLOCAL,
    /*
     * opcode: SETGLOBAL
     * Sets a global variable with the value of operand0 and identifier of operand1.
     * operand0: <src :: Register>
     * operand1: <id :: Identifier>
     * operand2: <>
     */
    SETGLOBAL,
    /*
     * opcode: LOADGLOBAL
     * Loads a global variable with identifier operand1 into operand0.
     * operand0: <dst :: Register>
     * operand1: <id :: Identifier>
     * operand2: <>
     */
    LOADGLOBAL,
    /*
     * opcode: LOADVAR
     * Loads a variable with identifier operand1 into operand0. Unwinds the stack to find the requested variable,
     *  unlike LOADLOCAL or LOADGLOBAL, which search in a predefined stack frame.
     * operand0: <>
     * operand1: <>
     * operand2: <>
     */
    LOADVAR,
    /*
     * opcode: ADDRR
     * Performs an add operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    ADDRR,
    /*
     * opcode: ADDRN
     * Performs an add operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Number>
     */
    ADDRN,
    /*
     * opcode: ADDRN
     * Performs an add operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Register>
     */
    ADDNR,
    /*
     * opcode: ADDNN
     * Performs an add operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Number>
     */
    ADDNN,
    /*
     * opcode: ADDIR
     * Performs an inline add operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <>
     */
    ADDIR,
    /*
     * opcode: ADDIN
     * Performs an inline add operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <>
     */
    ADDIN,
    /*
     * opcode: SUBRR
     * Performs a sub operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    SUBRR,
    /*
     * opcode: SUBRN
     * Performs a sub operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Number>
     */
    SUBRN,
    /*
     * opcode: SUBNR
     * Performs a sub operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Register>
     */
    SUBNR,
    /*
     * opcode: SUBNN
     * Performs a sub operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Number>
     */
    SUBNN,
    /*
     * opcode: SUBIR
     * Performs an inline sub operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <>
     */
    SUBIR,
    /*
     * opcode: SUBIN
     * Performs an inline sub operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <>
     */
    SUBIN,
    /*
     * opcode: MULRR
     * Performs a mul operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    MULRR,
    /*
     * opcode: MULRN
     * Performs a mul operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Number>
     */
    MULRN,
    /*
     * opcode: MULNR
     * Performs a mul operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Register>
     */
    MULNR,
    /*
     * opcode: MULNN
     * Performs a mul operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Number>
     */
    MULNN,
    /*
     * opcode: MULIR
     * Performs an inline mul operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <rhs :: Register>
     * operand2: <>
     */
    MULIR,
    /*
     * opcode: MULIN
     * Performs an inline mul operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <rhs :: Number>
     * operand2: <>
     */
    MULIN,
    /*
     * opcode: DIVRR
     * Performs a div operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    DIVRR,
    /*
     * opcode: DIVRN
     * Performs a div operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Number>
     */
    DIVRN,
    /*
     * opcode: DIVNR
     * Performs a div operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Register>
     */
    DIVNR,
    /*
     * opcode: DIVNN
     * Performs a div operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Number>
     */
    DIVNN,
    /*
     * opcode: DIVIR
     * Performs an inline div operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <rhs :: Register>
     * operand2: <>
     */
    DIVIR,
    /*
     * opcode: DIVIN
     * Performs an inline div operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <rhs :: Number>
     * operand2: <>
     */
    DIVIN,
    /*
     * opcode: POWRR
     * Performs a pow operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    POWRR,
    /*
     * opcode: POWRN
     * Performs a pow operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Number>
     */
    POWRN,
    /*
     * opcode: POWNR
     * Performs a pow operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Register>
     */
    POWNR,
    /*
     * opcode: POWNN
     * Performs a pow operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Number>
     */
    POWNN,
    /*
     * opcode: POWIR
     * Performs an inline pow operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <rhs :: Register>
     * operand2: <>
     */
    POWIR,
    /*
     * opcode: POWIN
     * Performs an inline pow operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <rhs :: Number>
     * operand2: <>
     */
    POWIN,
    /*
     * opcode: MODRR
     * Performs a mod operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    MODRR,
    /*
     * opcode: MODRN
     * Performs a mod operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Number>
     */
    MODRN,
    /*
     * opcode: MODNR
     * Performs a mod operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Register>
     */
    MODNR,
    /*
     * opcode: MODNN
     * Performs a mod operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Number>
     */
    MODNN,
    /*
     * opcode: MODIR
     * Performs an inline mod operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <rhs :: Register>
     * operand2: <>
     */
    MODIR,
    /*
     * opcode: MODIN
     * Performs an inline mod operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <rhs :: Number>
     * operand2: <>
     */
    MODIN,
    /*
     * opcode: NEGR
     * Negates the value in operand1 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <src :: Register>
     * operand2: <>
     */
    NEGR,
    /*
     * opcode: NEGI
     * Negates the value in operand0 in place.
     * operand0: <dst :: Register>
     * operand1: <>
     * operand2: <>
     */
    NEGI,
    /*
     * opcode: INC
     * Increments the value in operand0.
     * operand0: <dst :: Register>
     * operand1: <>
     * operand2: <>
     */
    INC,
    /*
     * opcode: DEC
     * Decrements the value in operand0.
     * operand0: <dst :: Register>
     * operand1: <>
     * operand2: <>
     */
    DEC,
    /*
     * opcode: BANDRR
     * Performs a bitwise AND operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    BANDRR,
    /*
     * opcode: BANDRN
     * Performs a bitwise AND operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Number>
     */
    BANDRN,
    /*
     * opcode: BANDNR
     * Performs a bitwise AND operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Register>
     */
    BANDNR,
    /*
     * opcode: BANDNN
     * Performs a bitwise AND operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Number>
     */
    BANDNN,
    /*
     * opcode: BANDIR
     * Performs an inline bitwise AND operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <rhs :: Register>
     * operand2: <>
     */
    BANDIR,
    /*
     * opcode: BANDIN
     * Performs an inline bitwise AND operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <rhs :: Number>
     * operand2: <>
     */
    BANDIN,
    /*
     * opcode: BORRR
     * Performs a bitwise OR operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    BORRR,
    /*
     * opcode: BORRN
     * Performs a bitwise OR operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Number>
     */
    BORRN,
    /*
     * opcode: BORNR
     * Performs a bitwise OR operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Register>
     */
    BORNR,
    /*
     * opcode: BORNN
     * Performs a bitwise OR operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Number>
     */
    BORNN,
    /*
     * opcode: BORIR
     * Performs an inline bitwise OR operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <rhs :: Register>
     * operand2: <>
     */
    BORIR,
    /*
     * opcode: BORIN
     * Performs an inline bitwise OR operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <rhs :: Boolean>
     * operand2: <>
     */
    BORIN,
    /*
     * opcode: BXORRR
     * Performs a bitwise XOR operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    BXORRR,
    /*
     * opcode: BXORRN
     * Performs a bitwise XOR operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Number>
     */
    BXORRN,
    /*
     * opcode: BXORNR
     * Performs a bitwise XOR operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Register>
     */
    BXORNR,
    /*
     * opcode: BXORNN
     * Performs a bitwise XOR operation between operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Number>
     * operand2: <rhs :: Number>
     */
    BXORNN,
    /*
     * opcode: BXORIR
     * Performs an inline bitwise XOR operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <rhs :: Register>
     * operand2: <>
     */
    BXORIR,
    /*
     * opcode: BXORIN
     * Performs an inline bitwise XOR operation between operand0 and operand1. Modifies the value of operand0.
     * operand0: <dst :: Register>
     * operand1: <rhs :: Number>
     * operand2: <>
     */
    BXORIN,
    BNOTR,
    BNOTI,
    /*
     * opcode: BSHLRR
     * Performs a bitwise shift left operation on operand1 by the amount specified in operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <shift :: Register>
     */
    BSHLRR,
    BSHLRN,
    BSHLNR,
    BSHLNN,
    BSHLIR,
    BSHLIN,
    /*
     * opcode: BSHRRR
     * Performs a bitwise shift right operation on operand1 by the amount specified in operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <shift :: Register>
     */
    BSHRRR,
    BSHRNR,
    BSHRRN,
    BSHRNN,
    BSHRIR,
    BSHRIN,
    /*
     * opcode: PUSHARG
     * Pushes value in operand0 onto the argument stack.
     * operand0: <src :: Register>
     * operand1: <>
     * operand2: <>
     */
    PUSHARG,
    /*
     * opcode: POPARG
     * Pops the top-most argument in the argument stack into operand0. Loads nil if the stack is empty.
     * operand0: <dst :: Register>
     * operand1: <>
     * operand2: <>
     */
    POPARG,
    /*
     * opcode: PUSHRET
     * Pushes the value of operand0 onto the return stack.
     * operand0: <src :: Register>
     * operand1: <>
     * operand2: <>
     */
    PUSHRET,
    /*
     * opcode: POPRET
     * Pops the top-most value in the return stack into operand0.
     * operand0: <src :: Register>
     * operand1: <>
     * operand2: <>
     */
    POPRET,
    /*
     * opcode: EQ
     * Performs a comparison operation between operand1 and operand2. Stores the result in operand0.
     * operand0: <src :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    EQ,
    /*
     * opcode: NEQ
     * Performs a reversed comparison operation between operand1 and operand2. Stores the result in operand0.
     * operand0: <src :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    NEQ,
    /*
     * opcode: LT
     * Performs a less-than comparison operation between operand1 and operand2. Stores the result in operand0.
     * operand0: <src :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    LT,
    /*
     * opcode: GT
     * Performs a greater-than comparison operation between operand1 and operand2. Stores the result in operand0.
     * operand0: <src :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    GT,
    /*
     * opcode: LE
     * Performs a less-than-or-equal-to comparison operation between operand1 and operand2. Stores the result in operand0.
     * operand0: <src :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    LE,
    /*
     * opcode: GE
     * Performs a greater-than-or-equal-to comparison operation between operand1 and operand2. Stores the result in operand0.
     * operand0: <src :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    GE,
    /*
     * opcode: JMP
     * Unconditinally jumps given offset in operand0. Does not save the state.
     * operand0: <off :: Number>
     * operand1: <>
     * operand2: <>
     */
    JMP,
    /*
     * opcode: JMPNZ
     * Jumps given offset in operand0 if the value in operand1 is not equal to 0. Does not save the state.
     * operand0: <off :: Number>
     * operand1: <cnd :: Register>
     * operand2: <>
     */
    JMPNZ,
    /*
     * opcode: JMPZ
     * Jumps given offset in operand0 if the value in operand1 is equal to 0. Does not save the state.
     * operand0: <off :: Number>
     * operand1: <cnd :: Register>
     * operand2: <>
     */
    JMPZ,
    /*
     * opcode: JMPEQ
     * Jumps given offset in operand0 if the value in operand1 is equal to the value in operand2. Does not save the state.
     * operand0: <off :: Number>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    JMPEQ,
    /*
     * opcode: JMPMEQ
     * Jumps given offset in operand0 if the value in operand1 is not equal to the value in operand2. Does not save the state.
     * operand0: <off :: Number>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    JMPNEQ,
    /*
     * opcode: JMPLT
     * Jumps given offset in operand0 if the value in operand1 is less than the value in operand2. Does not save the state.
     * operand0: <off :: Number>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    JMPLT,
    /*
     * opcode: JMPGT
     * Jumps given offset in operand0 if the value in operand1 greater than the value in operand2. Does not save the state.
     * operand0: <off :: Number>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    JMPGT,
    /*
     * opcode: JMPLE
     * Jumps given offset in operand0 if the value in operand1 is less than or equal to the value in operand2. Does not save the state.
     * operand0: <off :: Number>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    JMPLE,
    /*
     * opcode: JMPGE
     * Jumps given offset in operand0 if the value in operand1 is greater than or equal to the value in operand2. Does not save the state.
     * operand0: <off :: Number>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    JMPGE,
    /*
     * opcode: JMPLBL
     * Unconditionally jumps to label in operand0. Saves state.
     * operand0: <lbl :: Identifier>
     * operand1: <>
     * operand2: <>
     */
    JMPLBL,
    /*
     * opcode: JMPLBLNZ
     * Jumps to label in operand1 if the value in operand0 is not equal to 0. Saves state.
     * operand0: <lhs :: Register>
     * operand1: <lbl :: Identifier>
     * operand2: <>
     */
    JMPLBLNZ,
    /*
     * opcode: JMPLBLZ
     * Jumps to label in operand1 if the value in operand0 is equal to 0. Saves state.
     * operand0: <lhs :: Register>
     * operand1: <lbl :: Identifier>
     * operand2: <>
     */
    JMPLBLZ,
    /*
     * opcode: JMPLBLEQ
     * Jumps to label in operand2 if the value in operand0 is equal to the value in operand1. Saves state.
     * operand0: <lhs :: Register>
     * operand1: <rhs :: Register>
     * operand2: <lbl :: Identifier>
     */
    JMPLBLEQ,
    /*
     * opcode: JMPLBLEQ
     * Jumps to label in operand2 if the value in operand0 is not equal to the value in operand1. Saves state.
     * operand0: <lhs :: Register>
     * operand1: <rhs :: Register>
     * operand2: <lbl :: Identifier>
     */
    JMPLBLNEQ,
    /*
     * opcode: JMPLBLT
     * Jumps to label in operand2 if the value in operand0 is less than the value in operand1. Saves state.
     * operand0: <lhs :: Register>
     * operand1: <rhs :: Register>
     * operand2: <lbl :: Identifier>
     */
    JMPLBLLT,
    /*
     * opcode: JMPLBGT
     * Jumps to label in operand2 if the value in operand0 is greater than the value in operand1. Saves state.
     * operand0: <lhs :: Register>
     * operand1: <rhs :: Register>
     * operand2: <lbl :: Identifier>
     */
    JMPLBLGT,
    /*
     * opcode: JMPLBLE
     * Jumps to label in operand2 if the value in operand0 is less than or equal to the value in operand1. Saves state.
     * operand0: <lhs :: Register>
     * operand1: <rhs :: Register>
     * operand2: <lbl :: Identifier>
     */
    JMPLBLLE,
    /*
     * opcode: JMPLBGE
     * Jumps to label in operand2 if the value in operand0 is greater than or equal to the value in operand1. Saves state.
     * operand0: <lhs :: Register>
     * operand1: <rhs :: Register>
     * operand2: <lbl :: Identifier>
     */
    JMPLBLGE,
    /*
     * opcode: CALL
     * Calls the value in operand0 with arg count in operand1. Works for all callable types.
     * operand0: <callee :: Register>
     * operand1: <argc :: Number>
     * operand2: <>
     */
    CALL,
    /*
     * opcode: RET
     * Performs a return operation by jumping back to the return address of the current stack, and popping it.
     *  Only works inside function scopes.
     * operand0: <>
     * operand1: <>
     * operand2: <>
     */
    RET,
    /*
     * opcode: HALT
     * Halts the virtual machine with automatically deduced exit information.
     * operand0: <>
     * operand1: <>
     * operand2: <>
     */
    HALT,
    /*
     * opcode: EXIT
     * Performs an exit with the exit code in operand0 and an automatically deduced exit message.
     * operand0: <code :: Number>
     * operand1: <>
     * operand2: <>
     */
    EXIT,
    /*
     * opcode: STDOUT
     * Puts a value into standard output. Debug use only.
     * operand0: <src :: Register>
     * operand1: <>
     * operand2: <>
     */
    STDOUT,
    /*
     * opcode: STDIN
     * Puts standard input into operand0 as a String. Yields. Debug use only.
     * operand0: <dst :: Register>
     * operand1: <>
     * operand2: <>
     */
    STDIN,
    /*
     * opcode: LABEL
     * Declares a label with identifier operand0. Preprocessed opcode. Terminated by opcode NOP.
     * operand0: <lbl :: Identifier>
     * operand1: <>
     * operand2: <>
     */
    LABEL,
    /*
     * opcode: FUNC
     * Creates a function value and stores it in operand0. Saves the following opcodes as function body. Terminated by opcode NOP.
     * operand0: <dst :: Register>
     * operand1: <>
     * operand2: <>
     */
    FUNC,
    /*
     * opcode: LOADIDX
     * Loads the index operand2 of table operand1 into operand0.
     * operand0: <dst :: Register>
     * operand1: <tbl :: Register>
     * operand2: <idx :: Register>
     */
    LOADIDX,
    /*
     * opcode: SETIDX
     * Sets the index operand1 of table operand0 to value in operand2.
     * operand0: <tbl :: Register>
     * operand1: <idx :: Register>
     * operand2: <val :: Register>
     */
    SETIDX,
    /*
     * opcode: NEXT
     * Puts the next value inside table operand1 relative to the last invocation. Used for iteration.
     * operand0: <dst :: Register>
     * operand1: <tbl :: Register>
     * operand2: <>
     */
    NEXT,
    /*
     * opcode: LEN
     * Stores the length of the value in operand1 in operand0.
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
    /*
     * opcode: STRCONRR
     * Concatenates the strings in operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: Register>
     */
    STRCONRR,
    /*
     * opcode: STRCONRR
     * Concatenates the strings in operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: Register>
     * operand2: <rhs :: String>
     */
    STRCONRS,
    /*
     * opcode: STRCONRR
     * Concatenates the strings in operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: String>
     * operand2: <rhs :: Register>
     */
    STRCONSR,
    /*
     * opcode: STRCONRR
     * Concatenates the strings in operand1 and operand2 and stores the result in operand0.
     * operand0: <dst :: Register>
     * operand1: <lhs :: String>
     * operand2: <rhs :: String>
     */
    STRCONSS,
    /*
     * opcode: STRCONIR
     * Concatenates string in operand0 with operand1. Modifies operand0.
     * operand0: <lhs :: Register>
     * operand1: <rhs :: Register>
     * operand2: <>
     */
    STRCONIR,
    /*
     * opcode: STRCONIS
     * Concatenates string in operand0 with operand1. Modifies operand0.
     * operand0: <lhs :: Register>
     * operand1: <rhs :: String>
     * operand2: <>
     */
    STRCONIS,
    /*
     * opcode: STRIDX
     * Indexes into string in operand1 with operand2 and loads the value into operand0.
     * operand0: <dst :: Register>
     * operand1: <str :: Register>
     * operand2: <idx :: Register>
     */
    STRIDX,
    /*
     * opcode: STRSETIDX
     * Sets the index in operand1 of string stored in operand0 to value in operand2.
     * operand0: <str :: Register>
     * operand1: <idx :: Register>
     * operand2: <val :: Register>
     */
    STRSETIDX,
};

} // namespace via
