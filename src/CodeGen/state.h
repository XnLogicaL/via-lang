/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

namespace via {

struct JITState {
    std::unordered_map<std::string_view, int> symbol_table;
};

} // namespace via