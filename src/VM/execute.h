// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_EXECUTE_H
#define _VIA_EXECUTE_H

#include "common.h"
#include "state.h"

VIA_NAMESPACE_BEGIN

void execute(State* VIA_RESTRICT V);
void kill_thread(State* VIA_RESTRICT V);
void pause_thread(State* VIA_RESTRICT V);

VIA_NAMESPACE_END

#endif
