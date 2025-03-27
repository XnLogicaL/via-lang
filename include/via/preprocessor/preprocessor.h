// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_PREPROC_H
#define _VIA_PREPROC_H

#include "common.h"
#include "highlighter.h"
#include "token.h"

VIA_NAMESPACE_BEGIN

class Preprocessor final {
public:
    ~Preprocessor() = default;
    Preprocessor(ProgramData& program)
        : program(program),
          emitter(program) {}

    bool preprocess();
    void declare_default();

private:
    [[maybe_unused]] ProgramData& program;
    [[maybe_unused]] ErrorEmitter emitter;
};

VIA_NAMESPACE_END

#endif
