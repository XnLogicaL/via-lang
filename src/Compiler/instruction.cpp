// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "instruction.h"
#include "bitutils.h"
#include "bytecode.h"

namespace via {

using enum OpCode;

std::string to_string(const Bytecode &bytecode)
{
    static constexpr const char *format_string =
        "\033[0;35m{:<12}\033[0;37m {:<1} {:<1} {:<1}\033[0m\033[2m          {}\033[0m";

    const Instruction &instruction = bytecode.instruction;
    const InstructionData &data = bytecode.meta_data;

    OpCode opcode = instruction.op;
    U32 opcode_id = static_cast<U32>(opcode);

    if (opcode_id >= static_cast<U32>(JUMP) &&
        opcode_id <= static_cast<U32>(JUMPIFGREATEROREQUAL)) {
        return std::format(
            format_string,
            magic_enum::enum_name(opcode),
            static_cast<VIA_OPERAND_S>(instruction.operand0),
            static_cast<VIA_OPERAND_S>(instruction.operand1),
            static_cast<VIA_OPERAND_S>(instruction.operand2),
            data.comment.empty() ? "" : std::format("; {}", data.comment)
        );
    }
    else if (opcode == GETGLOBAL || opcode == SETGLOBAL) {
        return std::format(
            format_string,
            magic_enum::enum_name(opcode),
            instruction.operand0,
            U16_TO_U32(instruction.operand1, instruction.operand2),
            "",
            data.comment.empty() ? "" : std::format("; {}", data.comment)
        );
    }
    else {
        return std::format(
            format_string,
            magic_enum::enum_name(opcode),
            instruction.operand0,
            instruction.operand1,
            instruction.operand2,
            data.comment.empty() ? "" : std::format("; {}", data.comment)
        );
    }
}

} // namespace via
