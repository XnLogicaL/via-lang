# Spec XX: Interpreter

The interpreter accepts bytecode in the form of raw **Instructions**. The spec for it can be found in the [bytecode.md](./bytecode.md) spec.

## Subspec 00: State

The interpreter state (methods omitted) is defined as follows:
```cpp
struct State {
  Instruction*   pc;
  Instruction**  labels;
  CallStack*     callstack;
  ErrorState*    error;
  Dict*          globals;
  Value          returns;
  StkRegHolder&  stkregs;
  HeapRegHolder* heapregs;
}
```

### State::pc (Program counter)
---

Full definition:
```cpp
const Instruction* pc
```
*"A pointer to a constant instruction"*

`State::pc` is presumed by the interpreter to be a pointer view to the current instruction.

### State::labels (Label array)
---

Full definition:
```cpp
const Instruction* const* labels
```
*"A pointer to a constant array of constant instruction pointers"*

Labels are dynamically allocated during the interpreter's preprocessing stage, which occurs prior to execution. During this phase, the interpreter performs a linear scan over the bytecode of the entry point (typically the main function, as defined by the full binary file format), identifying all instructions of the form:

```cpp
LABEL <x>
```

Each label `<x>` is then mapped to its corresponding program counter within the program, and stored in the `State::labels` map.

The label map is allocated dynamically, as the total number and identifiers of labels are not known at compile time. This design allows for flexible and arbitrary use of labels within the bytecode without requiring a fixed-size label table.

### State::callstack
---

Full definition:
```cpp
CallStack* const callstack;
```

The callstack is a static stack-structure that holds **callframes**. It is defined as:
```cpp
struct CallStack {
  CallFrame frames[CALLSTACK_MAX_FRAMES];
  size_t frames_count;
};
```

Callframes are used to mark function calls. They hold references to the function call information, as well as a locally-managed stack for storing local variables.
Callframes are defined as the following:
```cpp
struct CallFrame {
  Closure* const closure;
  Value* locals;
  size_t locals_size;
  const Instruction* const savedpc;
}
```

## Subspec 01: Dispatching

The interpreter presumes that `state::pc` is a valid pointer to an instruction.

