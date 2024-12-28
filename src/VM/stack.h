/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include <cstdlib>
#include <stdexcept>

// Stack depth limit
#ifndef VIA_STACK_SIZE
#    define VIA_STACK_SIZE 128
#endif

namespace via
{

template<typename T>
struct TStack
{
    T *sp;  // Current top of the stack
    T *sbp; // Bottom of the stack
    size_t size;

    // Iterator class
    class Iterator
    {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T *;
        using reference = T &;

        explicit Iterator(T *ptr)
            : m_ptr(ptr)
        {
        }

        reference operator*() const
        {
            return *m_ptr;
        }
        pointer operator->()
        {
            return m_ptr;
        }

        // Pre-increment
        Iterator &operator++()
        {
            --m_ptr; // Move towards sbp (downward)
            return *this;
        }

        // Post-increment
        Iterator operator++(int)
        {
            Iterator temp = *this;
            --m_ptr;
            return temp;
        }

        // Pre-decrement
        Iterator &operator--()
        {
            ++m_ptr; // Move towards sp (upward)
            return *this;
        }

        // Post-decrement
        Iterator operator--(int)
        {
            Iterator temp = *this;
            ++m_ptr;
            return temp;
        }

        friend bool operator==(const Iterator &a, const Iterator &b)
        {
            return a.m_ptr == b.m_ptr;
        }

        friend bool operator!=(const Iterator &a, const Iterator &b)
        {
            return a.m_ptr != b.m_ptr;
        }

    private:
        T *m_ptr;
    };

    // Begin and end for range-based for loops
    Iterator begin()
    {
        return Iterator(sp);
    }
    Iterator end()
    {
        return Iterator(sbp - 1);
    }

    // Const iterators
    Iterator begin() const
    {
        return Iterator(sp);
    }
    Iterator end() const
    {
        return Iterator(sbp - 1);
    }
};

// Push a value onto the stack
template<typename T>
inline void tspush(TStack<T> *S, T val)
{
    *(++S->sp) = val;
}

// Pop a value from the stack
template<typename T>
inline void tspop(TStack<T> *S)
{
    S->sp--;
}

// Get the top value from the stack
template<typename T>
inline T tstop(TStack<T> *S)
{
    return *S->sp;
}

// Reset the stack pointer to the base
template<typename T>
inline void tsflush(TStack<T> *S)
{
    S->sp = S->sbp - 1;
}

// Create a new stack state
template<typename T>
TStack<T> *tsnewstate()
{
    // Allocate memory for the stack state
    auto *state = new TStack<T>;
    state->size = VIA_STACK_SIZE;

    // Allocate memory for the stack
    void *mem = std::malloc(state->size * sizeof(T));
    state->sbp = static_cast<T *>(mem);

    if (!state->sbp)
    {
        delete state;
        throw std::bad_alloc();
    }

    // Initialize the stack pointer
    state->sp = state->sbp - 1;

    return state;
}

// Delete a stack state
template<typename T>
void tscleanupstate(TStack<T> *S)
{
    std::free(S->sbp); // Free the stack memory
    delete S;          // Free the stack state
}

} // namespace via
