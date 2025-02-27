// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "instruction.h"
#include "bytecode.h"

namespace via {

std::string to_string(VIA_OPERAND_S operand)
{
    bool is_invalid = static_cast<VIA_OPERAND>(operand) == VIA_OPERAND_INVALID;
    std::string base = is_invalid ? "" : std::format("{:01}", operand);
    return base;
}

std::string to_string(const Bytecode &bytecode)
{
    std::string comment("");
    if (!bytecode.meta_data.comment.empty()) {
        comment = std::format("; {}", bytecode.meta_data.comment);
    }

    return std::format(
        "\033[0;35m{:<12}\033[0;37m {:<1} {:<1} {:<1}\033[0m\033[2m {:<15} {}\033[0m",
        magic_enum::enum_name(bytecode.instruction.op),
        to_string(static_cast<I32>(bytecode.instruction.operand0)),
        to_string(static_cast<I32>(bytecode.instruction.operand1)),
        to_string(static_cast<I32>(bytecode.instruction.operand2)),
        " ",
        comment
    );
}

} // namespace via
