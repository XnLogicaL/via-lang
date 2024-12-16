/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "instruction.h"
#include "opcode.h"
#include "optimizer.h"

#include <cmath>
#include <optional>

namespace via::Compilation
{

// Optimizes arithmetic statements (multiplication/division by a power of 2)
// by replacing them with bit shifts whenever possible
class BitShiftOptimizationPass : public OptimizationPass
{
public:
    void apply(Generator &, Bytecode &) override;

private:
    // Returns the exponent if `value` is a power of 2, otherwise std::nullopt
    std::optional<double> is_power_of_2(double);
};

} // namespace via::Compilation