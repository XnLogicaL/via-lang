// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_H_
#define VIA_H_

#include <ast/ast.h>
#include <ast/visitor.h>
#include <buffer.h>
#include <bytecode/builder.h>
#include <color.h>
#include <constexpr_ipow.h>
#include <constexpr_stof.h>
#include <constexpr_stoi.h>
#include <convert.h>
#include <diagnostics.h>
#include <iota.h>
#include <lexer/lexer.h>
#include <lexer/location.h>
#include <lexer/token.h>
#include <memory.h>
#include <module/defs.h>
#include <module/manager.h>
#include <module/module.h>
#include <module/symbol.h>
#include <panic.h>
#include <parser/parser.h>
#include <sema/const_value.h>
#include <sema/constexpr.h>
#include <sema/control_path.h>
#include <sema/register.h>
#include <semver.h>
#include <vm/header.h>
#include <vm/instruction.h>
#include <vm/interpreter.h>
#include <vm/stack.h>
#include <vm/value.h>
#include <vm/value_ref.h>
#include "policy.h"

#endif
