// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_H_
#define VIA_H_

#include <via-core/ast/ast.h>
#include <via-core/ast/visitor.h>
#include <via-core/buffer.h>
#include <via-core/color.h>
#include <via-core/constexpr_ipow.h>
#include <via-core/constexpr_stof.h>
#include <via-core/constexpr_stoi.h>
#include <via-core/convert.h>
#include <via-core/diagnostics.h>
#include <via-core/iota.h>
#include <via-core/lexer/lexer.h>
#include <via-core/lexer/location.h>
#include <via-core/lexer/token.h>
#include <via-core/memory.h>
#include <via-core/module.h>
#include <via-core/panic.h>
#include <via-core/parser/parser.h>
#include <via-core/sema/const_value.h>
#include <via-core/sema/constexpr.h>
#include <via-core/sema/context.h>
#include <via-core/sema/control_path.h>
#include <via-core/sema/register.h>
#include <via-core/semver.h>
#include <via-core/vm/header.h>
#include <via-core/vm/instruction.h>
#include <via-core/vm/interpreter.h>
#include <via-core/vm/stack.h>
#include <via-core/vm/value.h>
#include <via-core/vm/value_ref.h>
#include "policy.h"

#endif
