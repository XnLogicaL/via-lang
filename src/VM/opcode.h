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
    ADDINT,
    ADDFLOAT,
    SUB,
    SUBK,
    SUBINT,
    SUBFLOAT,
    MUL,
    MULK,
    MULINT,
    MULFLOAT,
    DIV,
    DIVK,
    DIVINT,
    DIVFLOAT,
    MOD,
    MODK,
    MODINT,
    MODFLOAT,
    POW,
    POWK,
    POWINT,
    POWFLOAT,
    NEG,
    NEGK,

    MOVE,
    SWAP,

    LOADK,
    LOADNIL,
    LOADINT,
    LOADFLOAT,
    LOADBOOL,
    LOADTABLE,
    LOADFUNCTION,

    PUSH,
    PUSHK,
    PUSHNIL,
    PUSHINT,
    PUSHFLOAT,
    PUSHBOOL,
    POP,
    GETSTACK,
    SETSTACK,
    GETARGUMENT,

    GETGLOBAL,
    SETGLOBAL,
    SETUPVALUE,
    GETUPVALUE,
    CAPTURE,

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
    JUMPIF,
    JUMPIFNOT,
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
