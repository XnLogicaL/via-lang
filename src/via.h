// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GPL v3.           |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "cache.h"
#include "encoder.h"
#include "signal.h"
#include "fileio.h"
#include "modifiable_once.h"
#include "callable_once.h"

#include "Lexer/lexer.h"
#include "Lexer/preproc.h"

#include "Parser/parser.h"
#include "Parser/ast.h"

#include "Compiler/compiler.h"
#include "Compiler/optimizer.h"

#include "VM/api.h"
#include "VM/state.h"
#include "VM/types.h"
#include "VM/register.h"
#include "VM/vlbase.h"
#include "VM/vltable.h"
#include "VM/vlmath.h"
#include "VM/vlrand.h"
#include "VM/vlthread.h"
#include "VM/vlfunction.h"
#include "VM/vlstring.h"
#include "VM/execute.h"
