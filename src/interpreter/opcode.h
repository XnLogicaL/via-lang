// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_OPCODE_H
#define _VIA_OPCODE_H

#include "common.h"

VIA_NAMESPACE_BEGIN

enum class OpCode : uint16_t {
    NOP,
    LABEL,
    EXIT,

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

    MOVE,
    SWAP,

    LOADK,
    LOADNIL,
    LOADINT,
    LOADFLOAT,
    LOADTRUE,
    LOADFALSE,
    LOADTABLE,
    LOADFUNCTION,
    LOADOBJECT,

    PUSH,
    PUSHK,
    PUSHNIL,
    PUSHINT,
    PUSHFLOAT,
    PUSHTRUE,
    PUSHFALSE,
    POP,
    DROP,
    GETSTACK,
    SETSTACK,
    GETARGUMENT,

    GETGLOBAL,
    SETGLOBAL,

    SETUPVALUE,
    GETUPVALUE,

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

    JUMPLABEL,
    JUMPLABELIF,
    JUMPLABELIFNOT,
    JUMPLABELIFEQUAL,
    JUMPLABELIFNOTEQUAL,
    JUMPLABELIFLESS,
    JUMPLABELIFGREATER,
    JUMPLABELIFLESSOREQUAL,
    JUMPLABELIFGREATEROREQUAL,

    CALL,
    EXTERNCALL,
    NATIVECALL,
    METHODCALL,
    RETURN,
    RETURNNIL,

    RAISE,
    TRY,
    CATCH,

    GETTABLE,
    SETTABLE,
    NEXTTABLE,
    LENTABLE,

    CONCAT,
    GETSTRING,
    SETSTRING,
    LENSTRING,

    TOINT,
    TOFLOAT,
    TOSTRING,
    TOBOOL,
};

VIA_NAMESPACE_END

#endif
