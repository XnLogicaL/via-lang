// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"

namespace via {

/*
 * OpCode operand convention
 * <opcode> <registers> <identifiers> <everything-else>
 * Ordered from most likely to be executed to the least. (Except NOP)
 */
enum class OpCode : U16 {
    NOP,
    ADD,
    ADDK,
    SUB,
    SUBK,
    MUL,
    MULK,
    DIV,
    DIVK,
    MOD,
    MODK,
    POW,
    POWK,
    NEG,
    NEGK,
    MOVE,
    SWAP,
    LOADK,
    LOADNIL,
    LOADTABLE,
    LOADFUNCTION,
    PUSH,
    PUSHK,
    POP,
    GETSTACK,
    SETSTACK,
    GETARGUMENT,
    GETGLOBAL,
    SETGLOBAL,
    INCREMENT,
    DECREMENT,
    EQUAL,
    NOTEQUAL,
    AND,
    OR,
    NOT,
    LESS,
    GREATER,
    LESSOREQUAL,
    GREATEROREQUAL,
    JUMP,
    JUMPIFNOT,
    JUMPIF,
    JUMPIFEQUAL,
    JUMPIFNOTEQUAL,
    JUMPIFLESS,
    JUMPIFGREATER,
    JUMPIFLESSOREQUAL,
    JUMPIFGREATEROREQUAL,
    CALL,
    EXTERNCALL,
    NATIVECALL,
    METHODCALL,
    RETURN,
    EXIT,
    GETTABLE,
    SETTABLE,
    NEXTTABLE,
    LENTABLE,
    CONCAT,
    CONCATK,
    CONCATI,
    GETSTRING,
    SETSTRING,
    LENSTRING,
    LEN,
    TYPEOF,
    TYPE,
    GET,
};

} // namespace via
