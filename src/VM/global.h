/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "shared.h"
#include "types.h"

namespace via
{

class Global
{
public:
    viaExitCode set_global(viaState *, viaGlobalIdentifier, viaValue);
    viaValue *get_global(viaState *, viaGlobalIdentifier);

private:
    viaHashMap<viaGlobalIdentifier, viaValue> consts;
};

} // namespace via
