/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "control.hpp"
#include <unordered_set>
#include "ast/ast.hpp"

std::vector<const via::ir::Term*> via::get_control_paths(const ir::StmtBlock* entry
) noexcept
{
    std::unordered_set<const ir::StmtBlock*> visited;
    std::vector<const ir::Term*> terms;
    std::function<void(const ir::StmtBlock*)> dfs = [&](const ir::StmtBlock* block) {
        if (!block || !visited.insert(block).second)
            return;

        if TRY_COERCE (const ir::TrReturn, ret, block->term) {
            terms.push_back(ret);
        } else if TRY_COERCE (const ir::TrBranch, br, block->term) {
            dfs(br->target);
        } else if TRY_COERCE (const ir::TrCondBranch, cbr, block->term) {
            dfs(cbr->iftrue);
            dfs(cbr->iffalse);
        } else {
            debug::bug("unmapped dfs block terminator");
        }
    };

    dfs(entry);
    return terms;
}
