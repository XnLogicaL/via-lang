// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "token.h"
#include "common.h"

namespace via {

class NodeVisitor;
struct ExprNode;
struct StmtNode;
struct TypeNode;

using pExprNode = std::unique_ptr<ExprNode>;
using pStmtNode = std::unique_ptr<StmtNode>;
using pTypeNode = std::unique_ptr<TypeNode>;

struct ExprNode {
    virtual ~ExprNode() = default;
    virtual std::string to_string() = 0;
    virtual void accept(NodeVisitor &, U32) = 0;
    virtual int precedence() const noexcept
    {
        return 0;
    };
};

struct StmtNode {
    virtual ~StmtNode() = default;
    virtual std::string to_string() = 0;
    virtual void accept(NodeVisitor &) = 0;
};

struct TypeNode {
    virtual ~TypeNode() = default;
    virtual std::string to_string() = 0;
    virtual void accept(NodeVisitor &) = 0;
};

} // namespace via