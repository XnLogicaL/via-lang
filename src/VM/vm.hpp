#pragma once

#ifndef via_vm_hpp
#define via_vm_hpp

#include "../common.hpp"

#include "chunk.hpp"
#include "debug.hpp"
#include "value.hpp"

#define STACK_MAX 256
#define HEAP_MAX 256

enum InterpretResult
{
    I_OK,
    I_COMPILE_ERR,
    I_RUNTIME_ERR,
};

struct VM
{
    Chunk* chunk;
    uint8_t* ip;
    Value stack[STACK_MAX];
    Value* stack_top;
    ValueArray heap[HEAP_MAX];

    void reset_stack()
    {
        this->stack_top = this->stack;
    }

    void push_stack(Value value)
    {
        *this->stack_top = value;
        this->stack_top++;
    }

    Value pop_stack()
    {
        this->stack_top--;
        return *this->stack_top;
    }

    void alloc_heap(Value value)
    {
        heap->write(value);
    }

    void init()
    {
        this->reset_stack();
    }

    void free()
    {

    }

    InterpretResult interpret(Chunk* chunk)
    {
        this->chunk = chunk;
        this->ip = chunk->opcode;
        return this->run();
    }

    InterpretResult run()
    {
#define READ_BYTE() (*this->ip++)
#define READ_CONST() (this->chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op) \
    do { \
        double b = this->pop_stack(); \
        double a = this->pop_stack(); \
        this->push_stack(a op b); \
    } while (false)

        for (;;)
        {

#ifdef DEBUG_TRACE_EXECUTION
    printf("          ");

    for (Value* slot = this->stack; slot < this->stack_top; slot++)
    {
        printf("[ ");
        print_value(*slot);
        printf(" ]");
    }

    printf("\n");

    disassemble_instruction(this->chunk, (int)(*this->ip - *this->chunk->opcode));
#endif
            uint8_t instruction;
            switch (instruction = READ_BYTE())
            {
            case OpCode::OP_RET: {
                print_value(this->pop_stack());
                printf("\n");
                return InterpretResult::I_OK;
            }

            case OpCode::OP_CONST: {
                auto constant = READ_CONST();
                this->push_stack(constant);
                print_value(constant);
                printf("\n");
                break;
            }

            case OpCode::OP_ADD:    BINARY_OP(+); break;
            case OpCode::OP_SUB:    BINARY_OP(-); break;
            case OpCode::OP_MUL:    BINARY_OP(*); break;
            case OpCode::OP_DIV:    BINARY_OP(/); break;
            case OpCode::OP_NEG:    this->push_stack(-this->pop_stack()); break;
            }
        }
#undef READ_BYTE
#undef READ_CONST
#undef BINARY_OP
    }
};

#endif