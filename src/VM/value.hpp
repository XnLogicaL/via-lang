#ifndef via_value_hpp
#define via_value_hpp

#include "../common.hpp"
#include "memory.hpp"

typedef double Value;

struct ValueArray
{
    int capacity;
    int count;
    Value* values;

    void init()
    {
        this->values = NULL;
        this->capacity = 0;
        this->count = 0;
    }

    void write(Value value)
    {
        if (this->capacity < this->count + 1) {
            int oldCapacity = this->capacity;
            this->capacity = GROW_CAPACITY(oldCapacity);
            this->values = GROW_ARRAY(Value, this->values,
                oldCapacity, this->capacity);
        }

        this->values[this->count] = value;
        this->count++;
    }

    void free()
    {
        FREE_ARRAY(Value, this->values, this->capacity);
        this->init();
    }
};

void print_value(Value value)
{
    printf("%g", value);
}

#endif