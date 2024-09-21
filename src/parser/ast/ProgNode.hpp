#pragma once

#include <vector>
#include <string>

#include "StmtNode.hpp"

struct ProgNode {
    std::vector<StmtNode*> stmts;
    std::string prog_name;
};
