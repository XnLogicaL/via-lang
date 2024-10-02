#ifndef via_debug_hpp
#define via_debug_hpp

#include "../common.hpp"
#include "chunk.hpp"
#include "value.hpp"

static int simple_instruction(const char* name, int offset)
{
    printf("%s\n", name);
    return offset + 1;
}

static int constant_instruction(const char* name, Chunk* chunk, int offset)
{
    uint8_t constant = chunk->opcode[offset + 1];
    printf("%-16s %4d '", name, constant);
    print_value(chunk->constants.values[constant]);
    printf("\n");

    return offset + 1;
}

int disassemble_instruction(Chunk* chunk, int offset)
{
    printf("%04d ", offset);

    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1])
        printf("   | ");
    else
        printf("%4d ", chunk->lines[offset]);

    uint8_t instruction = chunk->opcode[offset];

    switch (instruction)
    {
    case OpCode::OP_CONST:  return constant_instruction("OP_CONST", chunk, offset);
    case OpCode::OP_RET:    return simple_instruction("OP_RET", offset);
    case OpCode::OP_NEG:    return simple_instruction("OP_NEG", offset);
    case OpCode::OP_ADD:    return simple_instruction("OP_ADD", offset);
    case OpCode::OP_SUB:    return simple_instruction("OP_SUB", offset);
    case OpCode::OP_MUL:    return simple_instruction("OP_MUL", offset);
    case OpCode::OP_DIV:    return simple_instruction("OP_DIV", offset);
    default:
        std::cout << "Unknown instruction\n";
        return offset + 1;
    }
}

void disassemble_chunk(Chunk* chunk, const char* name)
{
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count;)
        offset = disassemble_instruction(chunk, offset);
}

#endif