// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_AST_BASE_H
#define _VIA_AST_BASE_H

#include "token.h"
#include "common.h"

VIA_NAMESPACE_BEGIN

class NodeVisitor;
struct ExprNode;
struct StmtNode;
struct TypeNode;

using pExprNode = std::unique_ptr<ExprNode>;
using pStmtNode = std::unique_ptr<StmtNode>;
using pTypeNode = std::unique_ptr<TypeNode>;

struct ExprNode {
    virtual VIA_DEFAULT_DESTRUCTOR(ExprNode);
    virtual std::string to_string(U32&)           = 0;
    virtual void        accept(NodeVisitor&, U32) = 0;
    virtual int         precedence() const noexcept {
        return 0;
    };
};

struct StmtNode {
    virtual VIA_DEFAULT_DESTRUCTOR(StmtNode);
    virtual std::string to_string(U32&)      = 0;
    virtual void        accept(NodeVisitor&) = 0;
};

struct TypeNode {
    virtual VIA_DEFAULT_DESTRUCTOR(TypeNode);
    virtual std::string to_string(U32&)      = 0;
    virtual void        accept(NodeVisitor&) = 0;
};

VIA_NAMESPACE_END

#endif
