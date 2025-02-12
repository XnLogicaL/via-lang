/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

#include "Lexer/lexer.h"
#include "Lexer/preproc.h"
#include "Lexer/cache.h"
#include "Lexer/encoder.h"

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
#include "VM/vlbit.h"
#include "VM/vlbuffer.h"
#include "VM/vlthread.h"
#include "VM/vlfunction.h"
#include "VM/vlstring.h"
#include "VM/vlbit.h"
#include "VM/execute.h"
