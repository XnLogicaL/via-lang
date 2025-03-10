// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "common.h"
#include "gc.h"
#include "instruction.h"
#include "opcode.h"
#include "state.h"
#include "strutils.h"
#include "rttypes.h"
#include "vmapi.h"
#include <cmath>

VIA_NAMESPACE_BEGIN

static const TValue nil = TValue();

TValue* get_register(State* V, U32 reg) {
    VIA_ASSERT(reg <= VIA_REGISTER_COUNT, "invalid register");
    return V->registers + reg;
}

void set_register(State* V, U32 reg, const TValue& val) {
    VIA_ASSERT(reg <= VIA_REGISTER_COUNT, "invalid register");

    TValue* addr = V->registers + reg;
    *addr        = val.clone();
}

// Returns the underlying pointer of a data type if present, nullptr if not.
void* to_pointer(const TValue& val) noexcept {
    return impl::__to_pointer(val);
}

// Returns whether if <val> has a heap component.
bool is_heap(const TValue& val) noexcept {
    return to_pointer(val) != nullptr;
}

// Compares 2 values and returns whether if they are equal.
// Optimized for maximum performance.
bool compare(const TValue& v0, const TValue& v1) noexcept {
    return impl::__compare(v0, v1);
};

// Pushes a copy of the given value onto the stack.
void push(State* VIA_RESTRICT V, const TValue& val) {
    VIA_ASSERT(V->sp < VIA_VM_STACK_SIZE / sizeof(TValue), "stack overflow");
    V->sbp[V->sp++] = val.clone();
}

// Pops a value from the stack and returns a copy of it.
TValue pop(State* VIA_RESTRICT V) {
    VIA_ASSERT(V->sp == 0, "stack underflow");
    return V->sbp[V->sp--].clone();
}

// Returns a copy of the top-most value on the stack.
TValue top(State* VIA_RESTRICT V) {
    VIA_ASSERT(V->sp == 0, "stack underflow");
    return V->sbp[V->sp--].clone();
}

// Returns a value that contains a String that represents the string-ified
// version of <val>. The return value is guaranteed to be a String type.
TValue to_string(State* VIA_RESTRICT V, const TValue& val) noexcept {
    using enum ValueType;

    if (check_string(val)) {
        return val.clone();
    }

    switch (val.type) {
    case integer: {
        std::string str  = std::to_string(val.val_integer);
        TString*    tstr = new TString(V, str.c_str());
        return TValue(tstr);
    }
    case floating_point: {
        std::string str  = std::to_string(val.val_integer);
        TString*    tstr = new TString(V, str.c_str());
        return TValue(tstr);
    }
    case boolean: {
        TString* str = new TString(V, val.val_boolean ? "true" : "false");
        return TValue(str);
    }
    case table: {
        std::string str = "{";

        for (auto& elem : val.cast_ptr<TTable>()->data) {
            str += to_string(V, elem.second).cast_ptr<TString>()->data;
            str += ", ";
        }

        if (str.back() == ' ') {
            str += "\b\b";
        }

        str += "}";

        TString* tstr = new TString(V, str.c_str());
        return TValue(tstr);
    }
    case function: {
        const void* faddr = val.cast_ptr<TFunction>();
        std::string str   = std::format("<function@{}>", faddr);
        TString*    tstr  = new TString(V, str.c_str());
        return TValue(tstr);
    }
    case cfunction: {
        // This has to be explicitly casted because function pointers be weird
        const void* cfaddr = val.cast_ptr<TCFunction>();
        std::string str    = std::format("<cfunction@{}>", cfaddr);
        TString*    tstr   = new TString(V, str.c_str());
        return TValue(tstr);
    }
    default:
        TString* tstr = new TString(V, "nil");
        return TValue(tstr);
    }

    VIA_UNREACHABLE;
    return via::nil.clone();
}

std::string to_cxx_string(State* VIA_RESTRICT V, const TValue& val) noexcept {
    TValue str = to_string(V, val);
    return std::string(str.cast_ptr<TString>()->data);
}

// Returns the truthiness of value <val>.
// Guaranteed to be a Bool type.
TValue to_bool(const TValue& val) noexcept {
    return impl::__to_bool(val);
}

bool to_cxx_bool(const TValue& val) noexcept {
    return impl::__to_cxx_bool(val);
}

// Returns the number representation of value <val>.
// Returns Nil if impossible, unlike `vtostring` or `vtobool`.
TValue to_number(const TValue& val) noexcept {
    return impl::__to_number(val);
}

template<typename T>
    requires std::is_arithmetic_v<T>
T to_cxx_number(const TValue& val) noexcept {
    return impl::__to_cxx_number<T>(val);
}

// Utility function for quick table indexing.
// Returns the value of key <key> if present in table <tbl>.
TValue get_table(TTable* VIA_RESTRICT tbl, U32 key, bool search_meta) noexcept {
    auto it = tbl->data.find(key);
    if (it != tbl->data.end()) {
        return it->second.clone();
    }
    else if (search_meta && tbl->meta) {
        // Disable metatable search to prevent chain searching
        // Which can cause infinite loops, crashes, and other bugs
        return get_table(tbl->meta, key, false);
    }

    // This has to be a pointer because we don't know if the value is a
    // non-pointer primitive type
    return nil.clone();
}

// Assigns the given value <val> to key <key> in table <tbl>.
void set_table(TTable* VIA_RESTRICT tbl, U32 key, const TValue& val) noexcept {
    if (check_nil(val)) {
        const TValue& tbl_val = get_table(tbl, key, false);

        if (!check_nil(tbl_val)) {
            tbl->data.erase(key);
        }
    }
    else {
        tbl->data.emplace(key, val.clone());
    }
}

// Utility function for quickly geting a metamethod associated to <op>.
TValue get_metamethod(const TValue& val, OpCode op) {
    if (!check_table(val)) {
        return nil.clone();
    }

#define GET_METHOD(id) (get_table(val.cast_ptr<TTable>(), hash_string_custom(id), true))
    switch (op) {
    case OpCode::ADD:
        return GET_METHOD("__add");
    case OpCode::SUB:
        return GET_METHOD("__sub");
    case OpCode::MUL:
        return GET_METHOD("__mul");
    case OpCode::DIV:
        return GET_METHOD("__div");
    case OpCode::POW:
        return GET_METHOD("__pow");
    case OpCode::MOD:
        return GET_METHOD("__mod");
    case OpCode::NEG:
        return GET_METHOD("__neg");
    case OpCode::INCREMENT:
        return GET_METHOD("__inc");
    case OpCode::DECREMENT:
        return GET_METHOD("__dec");
    case OpCode::CONCAT:
        return GET_METHOD("__con");
    default:
        VIA_ASSERT(false, "non-operator opcode");
        break;
    }

    return nil.clone();
#undef GET_METHOD
}

// Returns a local variable located at <offset>, relative to the stack base.
const TValue& get_local(State* VIA_RESTRICT V, U32 offset) noexcept {
    // Check if U32 is out of bounds
    if (offset > V->sp)
        return nil;

    TValue* stack_address = V->sbp + offset;
    TValue& val           = *stack_address;
    return val;
}

// Reassigns the stack value at offset <offset> to <val>.
void set_local(State* VIA_RESTRICT V, U32 offset, const TValue& val) {
    std::lock_guard<std::mutex> lock(V->G->symtable_mutex);

    // Check if U32 is out of bounds,
    if (offset > V->sp) {
        std::string identifier("<unknown-symbol>");
    }

    TValue* stack_address = V->sbp + offset;
    *stack_address        = val.clone();
}

// Returns the global with id <ident>, nil if it has not been declared.
const TValue& get_global(State* VIA_RESTRICT V, U32 ident) noexcept {
    std::lock_guard<std::mutex> lock(V->G->gtable_mutex);

    auto it = V->G->gtable.find(ident);
    if (it != V->G->gtable.end()) {
        return it->second;
    }

    return nil;
}

// Attempts to declare a new global constant.
void set_global(State* VIA_RESTRICT V, U32 ident, const TValue& val) {
    std::lock_guard<std::mutex> lock(V->G->gtable_mutex);

    auto it = V->G->gtable.find(ident);
    VIA_ASSERT(it == V->G->gtable.end(), "cannot reassign global");

    V->G->gtable.emplace(ident, val.clone());
}

// Returns the nth argument relative to the saved stack pointer of the current
// stack frame.
const TValue& get_argument(State* VIA_RESTRICT V, U32 offset) noexcept {
    // Check if the argument is out of bounds, return nil if so
    if (offset >= V->argc)
        return nil;

    // Calculate the stack position of the argument
    U32 stack_offset = V->ssp + V->argc - 1 - offset;
    // Retrieve stack value
    TValue& val = V->sbp[stack_offset];
    return val;
}

// Performs a native return operation, restores the stack and some other state
// information.
void native_return(State* VIA_RESTRICT V, SIZE retc) noexcept {
    std::vector<TValue> ret_values;
    // Restore state
    V->ip    = V->frame->ret_addr;
    V->frame = V->frame->caller;

    // Save return values
    for (SIZE i = 0; i < retc; i++) {
        TValue ret_val = pop(V);
        ret_values.push_back(std::move(ret_val));
    }

    // Restore stack pointer
    V->sp = V->ssp;

    // Clean up arguments
    for (SIZE i = 0; i < V->argc; i++)
        pop(V);

    // Restore return values
    for (int i = retc - 1; i >= 0; i--) // Reverse order for pushing return values
        push(V, ret_values[i]);
}

// Calls a native function.
void native_call(State* VIA_RESTRICT V, TFunction* VIA_RESTRICT callee, SIZE argc) noexcept {
    // Save state
    callee->caller   = V->frame;
    callee->ret_addr = V->ip;

    // Setup call information
    V->frame = callee;
    V->ip    = callee->bytecode;
    V->argc  = argc;
    V->ssp   = V->sp;
}

// Calls a C function pointer.
// Mimics stack behavior as it would behave while calling a native function.
void extern_call(State* VIA_RESTRICT V, TCFunction* VIA_RESTRICT cf, SIZE argc) noexcept {
    /* Stack allocate id string
        15 additional characters:
        +9 for 'cfunction'
        +1 for '@'
        +2 for '0x'
        +2 for '<' and '>'
        +1 for '\0'
    */
    char buf[2 * sizeof(void*) + 15];
    // Implicitly cast into const void * for formatting
    const void* addr = cf;
    std::snprintf(buf, sizeof(buf), "<cfunction@0x%p>", addr);

    TFunction func{

    };

    native_call(V, &func, argc);
    // Call function pointer
    cf->data(V);
}

// Calls a table method.
void method_call(State* VIA_RESTRICT V, TTable* VIA_RESTRICT tbl, U32 key, SIZE argc) noexcept {
    const TValue& method = get_table(tbl, key, true);

    if (check_function(method)) {
        native_call(V, method.cast_ptr<TFunction>(), argc);
    }
    else if (check_cfunction(method)) {
        extern_call(V, method.cast_ptr<TCFunction>(), argc);
    }
    else {
        VIA_ASSERT(false, "value is not callable");
    }
}

// Returns the primitive type of value <val>.
TValue type(State* VIA_RESTRICT V, const TValue& val) noexcept {
    char* str = duplicate_string(std::string(magic_enum::enum_name(val.type)));
    return TValue(new TString(V, str));
}

// Unified call interface.
// Works on all callable types (TFunction, TCFunction, TTable).
void call(State* VIA_RESTRICT V, const TValue& val, SIZE argc) noexcept {
    V->calltype = CallType::CALL;

    if (check_function(val)) {
        native_call(V, val.cast_ptr<TFunction>(), argc);
    }
    else if (check_cfunction(val)) {
        extern_call(V, val.cast_ptr<TCFunction>(), argc);
    }
    else if (check_table(val)) {
        method_call(V, val.cast_ptr<TTable>(), hash_string_custom("__call"), argc);
    }
    else {
        VIA_ASSERT(false, "value is not callable");
    }
}

// Returns the length of value <val>, nil if impossible.
TValue len(State* VIA_RESTRICT V, const TValue& val) noexcept {
    if (check_string(val)) {
        return TValue(static_cast<int>(strlen(val.cast_ptr<TString>()->data)));
    }
    else if (check_table(val)) {
        U32           metamethod_key = hash_string_custom("__len");
        const TValue& metamethod     = get_table(val.cast_ptr<TTable>(), metamethod_key, true);

        if (check_nil(metamethod)) {
            return TValue(static_cast<int>(val.cast_ptr<TTable>()->data.size()));
        }

        call(V, metamethod, 1);
        return pop(V);
    }

    return nil.clone();
}

// Returns the complex type of value <val>.
// Practically the same as `type()`, but returns
// the `__type` key if the given table has it defined.
TValue typeofv(State* VIA_RESTRICT V, const TValue& val) noexcept {
    if (check_table(val)) {
        TTable*       tbl = val.cast_ptr<TTable>();
        const TValue& ty  = get_table(tbl, hash_string_custom("__type"), true);
        // Check if the __type property is Nil
        // if so return the primitive type
        if (check_nil(ty)) {
            return type(V, val);
        }

        TString* tystr = new TString(V, ty.cast_ptr<TString>()->data);
        return TValue(tystr);
    }

    return type(V, val);
}

TValue weak_primitive_cast(State* VIA_RESTRICT V, const TValue& val, ValueType type) {
    using enum ValueType;

    switch (type) {
    case integer:
    case floating_point:
        return to_number(val);
    case boolean:
        return to_bool(val);
    case string:
        return to_string(V, val);
    default:
        break;
    }

    return via::nil.clone();
}

void strong_primitive_cast(State* VIA_RESTRICT V, TValue& val, ValueType type) {
    using enum ValueType;

    switch (type) {
    case floating_point: {
        float num = to_cxx_number<float>(val);
        // Check for NaN
        if (num != num) {
            goto error;
        }

        val.val_floating_point = num;
        break;
    }
    case integer: {
        int num         = to_cxx_number<int>(val);
        val.val_integer = num;
        break;
    }
    case boolean:
        val.val_boolean = to_cxx_bool(val);
        break;
    case string: {
        TValue   non_owned_val = to_string(V, val);
        TString* owned         = val.cast_ptr<TString>();

        val.val_pointer = new TString(V, non_owned_val.cast_ptr<TString>()->data);

        if (owned) {
            delete owned;
        }

        break;
    }
    default:
        goto error;
    }

    val.type = type;
    return;
error:
    VIA_ASSERT(
        false,
        std::format(
            "type '{}' is not primitive castable into type '{}'",
            magic_enum::enum_name(val.type),
            magic_enum::enum_name(type)
        )
    );
}

VIA_NAMESPACE_END
