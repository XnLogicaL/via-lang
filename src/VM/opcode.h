#ifndef VIA_OPCODE_H
#define VIA_OPCODE_H

namespace via
{

namespace VM
{

enum class OpCode {
    NOP,  // No operation

    // Register Operations
    MOV,        // Move value from one register to another <dst :: Register> <src :: Register>
    LOAD,       // Load value from memory into a register <dst :: Register> <addr :: Integer>
    STORE,      // Store value from a register to memory <addr :: Integer> <src :: Register>
    LI,         // Load immediate value into register <dst :: Register> <val :: Any>
    DUP,        // Duplicate a register value
    SWAP,       // Swap values between two registers

    // Register Manipulation
    PUSH,       // Push a new stack frame onto the stack
    POP,        // Pop a stack frame from the stack
    SETLOCAL,   // Set a new local to the top stack frame
    GETLOCAL,   // Load a local from the top stack frame

    // Arithmetic Operations
    ADD,        // Add two registers and store result in the first register
    SUB,        // Subtract second register from the first and store result
    MUL,        // Multiply two registers and store result in the first register
    DIV,        // Divide first register by the second and store result
    MOD,        // Modulus of first register by the second and store result
    NEG,        // Negate the value in a register

    // Logical Operations
    AND,        // Bitwise AND between two registers
    OR,         // Bitwise OR between two registers
    XOR,        // Bitwise XOR between two registers
    NOT,        // Bitwise NOT of a register

    // Comparison Operations
    EQ,         // Check if two registers are equal
    NEQ,        // Check if two registers are not equal
    LT,         // Check if first register is less than the second
    GT,         // Check if first register is greater than the second
    LE,         // Check if first register is less than or equal to the second
    GE,         // Check if first register is greater than or equal to the second

    // Control Flow
    JMP,        // Unconditional jump to an instruction address
    JNZ,        // Jump if the value in the specified register is non-zero
    JZ,         // Jump if the value in the specified register is zero
    CALL,       // Call function at a specified address
    CALLC,      // Call function with C-style calling convention
    RET,        // Return from a function
    HALT,       // Halt execution
    EXIT,       // Exit with a custom exit code

    // Input/Output Operations
    STDOUT,     // Output value from a register to standard output
    STDIN,      // Input value from standard input into a register

    // Garbage Collection Operations
    GCADD,      // Add (malloc-ed) pointer to garbage collector free list
    GCCOL,      // Invoke garbage collection

    // Label, Function, and End Operations
    LABEL,      // Define a label for a jump target
    FUNC,       // Define a function
    END,        // End the current function or block

    // Insert and Freeze Operations
    INSERT,     // Insert a value into a collection or structure
    FREEZE,     // Freeze a value to prevent further modification
    ISFROZEN,   // Check if a value is frozen
    CALLM,      // Call a method (on an object or class)
    
    // Index Operations
    LOADIDX,    // Load a value from a specified index
    SETIDX,     // Set a value at a specified index
    LEN,        // Get the length of a collection or string

    TOSTRING,   // Converts a value into a string, "nil" if impossible
    TONUMBER,   // Converts a value into a number, nil if impossible
    TOBOOL,     // Converts a value into a bool (truthiness)

    // File System Operations
    FSWRITE,    // Write data to a file
    FSREAD,     // Read data from a file
    FSMKDIR,
    FSRM,

    // Type Operations
    TYPEOF,     // Get the complex type of a value (e.g., class, table)
    TYPE,       // Get the primitive type of a value (e.g., INT, STRING)
    ISNIL,      // Check if a value is nil

    // String Operations
    STRCON,     // Concatenate two strings
    STRSUB,     // Get a substring from a string
    STRUP,
    STRLOW,

    // Memory Operations
    ALLOC,      // Allocate memory for a variable
    FREE,       // Free allocated memory
    MEMCPY,     // Copy memory from one location to another

    // Thread Operations
    THRDCREATE,  // Create a new thread
    THRDJOIN,    // Wait for a thread to finish
    THRDEXIT,    // Exit a thread
    THRDID,      // Get the ID of the current thread
};

} // namespace VM
    
} // namespace via

#endif // VIA_OPCODE_H
