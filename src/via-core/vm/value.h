/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.h>
#include <via/types.h>
#include "interpreter.h"
#include "option.h"
#include "sema/const_value.h"

namespace via
{

class Value final
{
 public:
  using int_type = i64;
  using float_type = f64;

  enum class Kind
  {
    NIL,
    INT,
    FLOAT,
    BOOL,
    STRING,
  };

  union Union
  {
    int_type int_;
    float_type float_;
    bool boolean;
    char* string;
  };

  friend class ValueRef;
  friend class Interpreter;

  template <bool, bool>
  friend void __execute(Interpreter*);

 public:
  static inline Value* construct(Interpreter* ctx)
  {
    return constructImpl(ctx, Kind::NIL);
  }

  static inline Value* construct(Interpreter* ctx, Value::int_type int_)
  {
    return constructImpl(ctx, Kind::INT, {.int_ = int_});
  }

  static inline Value* construct(Interpreter* ctx, Value::float_type float_)
  {
    return constructImpl(ctx, Kind::FLOAT, {.float_ = float_});
  }

  static inline Value* construct(Interpreter* ctx, bool boolean)
  {
    return constructImpl(ctx, Kind::BOOL, {.boolean = boolean});
  }

  static inline Value* construct(Interpreter* ctx, char* string)
  {
    debug::require(
      ctx->getAllocator().owns(string),
      "Value construction via a string literal requires it to be allocated by "
      "the corresponding Value::ctx");
    return constructImpl(ctx, Kind::STRING, {.string = string});
  }

  static inline Value* construct(Interpreter* ctx, const sema::ConstValue& cv)
  {
    using enum sema::ConstValue::Kind;

    auto& alloc = ctx->getAllocator();

    switch (cv.kind()) {
      case NIL:
        return construct(ctx);
      case BOOL:
        return construct(ctx, cv.value<BOOL>());
      case INT:
        return construct(ctx, cv.value<INT>());
      case FLOAT:
        return construct(ctx, cv.value<FLOAT>());
      case STRING: {
        auto buf = alloc.strdup(cv.value<STRING>().c_str());
        return construct(ctx, buf);
      }
      default:
        break;
    }

    debug::unimplemented();
  }

 public:
  inline auto kind() const { return mKind; }
  inline auto& data() { return mData; }
  inline const auto& data() const { return mData; }
  inline auto* context() const { return mCtx; }

  inline void free()
  {
    switch (mKind) {
      case Kind::STRING:
        mCtx->getAllocator().free(mData.string);
        break;
      default:
        // Trivial types don't require explicit destruction
        break;
    }

    mKind = Kind::NIL;
  }

  inline Value* clone() { return constructImpl(mCtx, mKind, mData); }

  // Totally safe access methods
  inline int_type getInt() const { return mData.int_; }
  inline float_type getFloat() const { return mData.float_; }
  inline bool getBool() const { return mData.boolean; }
  inline char* getString() const { return mData.string; }

  inline Option<int_type> asCInt() const { return nullopt; /* PLACEHOLDER */ }
  inline Option<float_type> asCFloat() const
  {
    return nullopt; /* PLACEHOLDER */
  }
  inline bool asCBool() const { return false; /* PLACEHOLDER */ }
  inline char* asCString() const { return nullptr; /* PLACEHOLDER */ }

  inline Value* asInt() const { return nullptr; /* PLACEHOLDER */ }
  inline Value* asFloat() const { return nullptr; /* PLACEHOLDER */ }
  inline Value* asBool() const { return nullptr; /* PLACEHOLDER */ }
  inline Value* asString() const { return nullptr; /* PLACEHOLDER */ }

 private:
  static inline Value* constructImpl(Interpreter* ctx,
                                     Value::Kind kind,
                                     Value::Union data = {})
  {
    Value* ptr = ctx->getAllocator().emplace<Value>();
    ptr->mKind = kind;
    ptr->mData = data;
    ptr->mCtx = ctx;
    return ptr;
  }

 private:
  Kind mKind = Kind::NIL;
  Union mData = {};
  usize mRc = 0;
  Interpreter* mCtx = nullptr;
};

}  // namespace via
