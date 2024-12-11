/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

namespace via
{

enum class OpCode
{
    ERR, // Error opcode
    NOP, // No operation

    // Register Operations
    MOV, // Move value from one register to another <dst :: viaRegister> <src :: viaRegister>
    CPY,
    LI,   // Load immediate value into register <dst :: viaRegister> <val :: Any>
    DUP,  // Duplicate a register value
    SWAP, // Swap values between two registers
    NIL,  // Sets a register to nil

    // Register Manipulation
    PUSH,       // Push a new stack frame onto the stack
    POP,        // Pop a stack frame from the stack
    SETLOCAL,   // Set a new local to the top stack frame
    LOADLOCAL,  // Load a local from the top stack frame
    SETGLOBAL,  // Set a global to the global environment, will fail if the identifier is already defined
    LOADGLOBAL, // Load a global from the global environment
    LOADVAR,    // First searches for the variable in the global scope, then unwinds the callstack to find it, and finally searches the global scope

    // Arithmetic Operations
    ADD, // Add two registers and store result in the first register
    SUB, // Subtract second register from the first and store result
    MUL, // Multiply two registers and store result in the first register
    DIV, // Divide first register by the second and store result
    MOD, // Modulus of first register by the second and store result
    POW,
    IADD,
    ISUB,
    IMUL,
    IDIV,
    IMOD,
    IPOW,
    NEG, // Negate the value in a register
    INC,
    DEC,

    // Logical Operations
    BAND, // Bitwise AND between two registers
    BOR,  // Bitwise OR between two registers
    BXOR, // Bitwise XOR between two registers
    BNOT, // Bitwise NOT of a register
    BSHL, // Bitwise SHL, performed on number only
    BSHR, // Bitwise SHR, performed on number only
    BSAR,
    BROL,
    BROR,

    // Stack operations
    PUSHARG,
    POPARG,
    PUSHRET,
    POPRET,

    // Comparison Operations
    EQ,  // Check if two registers are equal
    NEQ, // Check if two registers are not equal
    LT,  // Check if first register is less than the second
    GT,  // Check if first register is greater than the second
    LE,  // Check if first register is less than or equal to the second
    GE,  // Check if first register is greater than or equal to the second

    // Control Flow
    JMP,   // Unconditional jump to an instruction address
    JNZ,   // Jump if the value in the specified register is non-zero
    JZ,    // Jump if the value in the specified register is zero
    JEQ,   // Jump if the given registers hold the same value
    JNEQ,  // Jump if the given registers don't the same value
    JLT,   // Jump if the first register is less than the second register
    JGT,   // Jump if the first register is greater than the second register
    JNLT,  // Jump if not less than
    JNGT,  // Jump if not greater than
    JLE,   // Jump if the first register is less than or equal to the second register
    JNLE,  // Jump if not less than or equal to
    JGE,   // Jump if the first register is greater than or equal to the second register
    JNGE,  // Jump if not greater than or equal to
    JL,    // Jump to label
    JLNZ,  // Jump to label if not zero
    JLZ,   // Jump to label if zero
    JLEQ,  // Jump to label if equal to
    JLNEQ, // Jump to label if not equal to
    JLLT,  // Jump to label if less than
    JLNLT, // Jump to label if not less than
    JLGT,  // Jump to label if greater than
    JLNGT, // Jump to label if not greater than
    JLLE,  // Jump to label if less than or equal to
    JLNLE, // Jump to label if not less than or equal to
    JLGE,  // Jump to label if greater than or equal to
    JLNGE, // Jump to label if n not greate than or equal to
    CALL,  // Call function at a specified address
    CALLM, // Call a method (on an object or class)
    RET,   // Return from a function
    HALT,  // Halt execution
    EXIT,  // Exit with a custom exit code

    // Input/Output Operations
    STDOUT, // Output value from a register to standard output
    STDIN,  // Input value from standard input into a register

    // Garbage Collection Operations
    GCADD, // Add (malloc-ed) pointer to garbage collector free list
    GCCOL, // Invoke garbage collection

    // Label, Function, and End Operations
    LABEL, // Define a label for a jump target
    FUNC,  // Define a function
    END,   // End the current function or block

    // Index Operations
    LOADIDX, // Load a value from a specified index
    SETIDX,  // Set a value at a specified index
    LEN,     // Get the length of a collection or string

    TOSTRING, // Converts a value into a string, "nil" if impossible
    TONUMBER, // Converts a value into a number, nil if impossible
    TOBOOL,   // Converts a value into a bool (truthiness)

    // Type Operations
    TYPEOF, // Get the complex type of a value (e.g., class, table)
    TYPE,   // Get the primitive type of a value (e.g., INT, STRING)
    ISNIL,  // Check if a value is nil

    // String Operations
    STRCON, // Concatenate two strings

    // Debug OpCodes
    DEBUGREGISTERS,
    DEBUGARGUMENTS,
    DEBUGRETURNS,
};

} // namespace via
