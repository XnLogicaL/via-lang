// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "ast.h"
#include "register_allocator.h"
#include "token.h"
#include "common.h"
#include "types.h"
#include "bytecode.h"
#include "constant.h"
#include "highlighter.h"

#define INVALID_VISIT \
    { \
        VIA_ASSERT(false, "invalid visit"); \
    }

#define PUSH_K(constant) program->constants->push_constant(constant);
#define IS_INHERITOR(obj, inheritor) \
    (std::type_index(typeid(obj)) == std::type_index(typeid(inheritor)))
#define IS_CONSTEXPR(obj) IS_INHERITOR(obj, LiteralNode)

namespace via {

VIA_INLINE TValue construct_constant(LiteralNode &literal_node)
{
    using enum ValueType;
    return std::visit(
        [](auto &&val) -> TValue {
            using T = std::decay_t<decltype(val)>;

            if constexpr (std::is_same_v<T, int>) {
                return TValue(val);
            }
            else if constexpr (std::is_same_v<T, bool>) {
                return TValue(val);
            }
            else if constexpr (std::is_same_v<T, float>) {
                return TValue(val);
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                TString *tstring = new TString(nullptr, val.data());
                return TValue(string, static_cast<void *>(tstring));
            }

            VIA_UNREACHABLE;
        },
        literal_node.value
    );
}

class NodeVisitor {
public:
    virtual ~NodeVisitor() = default;

    virtual void visit(LiteralNode &, U32) INVALID_VISIT;
    virtual void visit(VariableNode &, U32) INVALID_VISIT;
    virtual void visit(UnaryNode &, U32) INVALID_VISIT;
    virtual void visit(GroupNode &, U32) INVALID_VISIT;
    virtual void visit(CallNode &, U32) INVALID_VISIT;
    virtual void visit(IndexNode &, U32) INVALID_VISIT;
    virtual void visit(BinaryNode &, U32) INVALID_VISIT;

    virtual void visit(DeclarationNode &) INVALID_VISIT;
    virtual void visit(ScopeNode &) INVALID_VISIT;
    virtual void visit(FunctionNode &) INVALID_VISIT;
    virtual void visit(AssignNode &) INVALID_VISIT;
    virtual void visit(IfNode &) INVALID_VISIT;
    virtual void visit(WhileNode &) INVALID_VISIT;
    virtual void visit(ExprStmtNode &) INVALID_VISIT;

    inline bool failed()
    {
        return visitor_failed;
    }

protected:
    bool visitor_failed = false;
};

class ExprVisitor : public NodeVisitor {
public:
    ExprVisitor(ProgramData *program, Emitter &emitter, RegisterAllocator &allocator)
        : program(program)
        , emitter(emitter)
        , allocator(allocator)
    {
    }

    void visit(LiteralNode &, U32) override;
    void visit(VariableNode &, U32) override;
    void visit(UnaryNode &, U32) override;
    void visit(GroupNode &, U32) override;
    void visit(CallNode &, U32) override;
    void visit(IndexNode &, U32) override;
    void visit(BinaryNode &, U32) override;

private:
    ProgramData *program;
    Emitter &emitter;
    RegisterAllocator &allocator;
};

class StmtVisitor : public NodeVisitor {
public:
    StmtVisitor(ProgramData *program, Emitter &emitter, RegisterAllocator &allocator)
        : program(program)
        , emitter(emitter)
        , allocator(allocator)
        , expression_visitor(program, emitter, allocator)
    {
    }

    void visit(DeclarationNode &) override;
    void visit(ScopeNode &) override;
    void visit(FunctionNode &) override;
    void visit(AssignNode &) override;
    void visit(IfNode &) override;
    void visit(WhileNode &) override;
    void visit(ExprStmtNode &) override;

private:
    ProgramData *program;
    Emitter &emitter;
    RegisterAllocator &allocator;
    ExprVisitor expression_visitor;
};

class PrintVisitor : public NodeVisitor {
public:
    void visit(LiteralNode &, U32) override;
    void visit(VariableNode &, U32) override;
    void visit(UnaryNode &, U32) override;
    void visit(GroupNode &, U32) override;
    void visit(BinaryNode &, U32) override;
};

} // namespace via
