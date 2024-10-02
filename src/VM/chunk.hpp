#pragma once

#ifndef via_bytecode_hpp
#define via_bytecode_hpp

#include "../common.hpp"
#include "memory.hpp"
#include "value.hpp"

enum OpCode
{
    OP_NULL,
    OP_CONST,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_NEG,
    OP_RET,
};

struct Chunk
{
    uint8_t* opcode;
    int* lines;
    int capacity;
    int count;
    ValueArray constants;

    void init()
    {
        this->count = 0;
        this->capacity = 0;
        this->lines = NULL;
        this->opcode = NULL;
        this->constants.init();
    }

    void write(uint8_t byte, int line)
    {
        if (this->capacity < this->count + 1) {
            int old_capacity = this->capacity;

            this->capacity = GROW_CAPACITY(old_capacity);
            this->opcode = GROW_ARRAY(uint8_t, this->opcode,
                old_capacity, this->capacity);
            this->lines = GROW_ARRAY(int, this->lines,
                old_capacity, this->capacity
            );
        }

        this->opcode[this->count] = byte;
        this->lines[this->count] = line;
        this->count++;
    }

    void free()
    {
        FREE_ARRAY(uint8_t, this->opcode, this->capacity);
        FREE_ARRAY(int, this->lines, this->capacity);
        this->constants.free();
        this->init();
    }

    int push_constant(Value value)
    {
        this->constants.write(value);
        return this->constants.count - 1;
    }
};

#endif