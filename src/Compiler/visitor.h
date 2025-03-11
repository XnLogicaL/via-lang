// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_VISITOR_H
#define _VIA_VISITOR_H

#include "ast.h"
#include "register_allocator.h"
#include "token.h"
#include "common.h"
#include "rttypes.h"
#include "bytecode.h"
#include "constant.h"
#include "highlighter.h"

#define INVALID_VISIT                                                                              \
    {                                                                                              \
        VIA_ASSERT(false, "invalid visit");                                                        \
    }

// =============================================================================================
// visitor.h
//
VIA_NAMESPACE_BEGIN

TValue construct_constant(LiteralNode&);

class NodeVisitor {
public:
    virtual VIA_DEFAULT_DESTRUCTOR(NodeVisitor);

    virtual void visit(LiteralNode&, Operand) INVALID_VISIT;
    virtual void visit(SymbolNode&, Operand) INVALID_VISIT;
    virtual void visit(UnaryNode&, Operand) INVALID_VISIT;
    virtual void visit(GroupNode&, Operand) INVALID_VISIT;
    virtual void visit(CallNode&, Operand) INVALID_VISIT;
    virtual void visit(IndexNode&, Operand) INVALID_VISIT;
    virtual void visit(BinaryNode&, Operand) INVALID_VISIT;

    virtual pTypeNode visit(AutoNode&) INVALID_VISIT;
    virtual pTypeNode visit(GenericNode&) INVALID_VISIT;
    virtual pTypeNode visit(UnionNode&) INVALID_VISIT;
    virtual pTypeNode visit(FunctionTypeNode&) INVALID_VISIT;
    virtual pTypeNode visit(AggregateNode&) INVALID_VISIT;

    virtual void visit(DeclarationNode&) INVALID_VISIT;
    virtual void visit(ScopeNode&) INVALID_VISIT;
    virtual void visit(FunctionNode&) INVALID_VISIT;
    virtual void visit(AssignNode&) INVALID_VISIT;
    virtual void visit(IfNode&) INVALID_VISIT;
    virtual void visit(WhileNode&) INVALID_VISIT;
    virtual void visit(ExprStmtNode&) INVALID_VISIT;

    virtual inline bool failed() {
        return visitor_failed;
    }

protected:
    bool visitor_failed = false;
};

#undef INVALID_VISIT

class ExprVisitor : public NodeVisitor {
public:
    ExprVisitor(ProgramData& program, Emitter& emitter, RegisterAllocator& allocator)
        : program(program),
          emitter(emitter),
          allocator(allocator) {}

    void visit(LiteralNode&, Operand) override;
    void visit(SymbolNode&, Operand) override;
    void visit(UnaryNode&, Operand) override;
    void visit(GroupNode&, Operand) override;
    void visit(CallNode&, Operand) override;
    void visit(IndexNode&, Operand) override;
    void visit(BinaryNode&, Operand) override;

private:
    ProgramData&       program;
    Emitter&           emitter;
    RegisterAllocator& allocator;
};

class DecayVisitor : public NodeVisitor {
public:
    DecayVisitor(ProgramData& program, Emitter& emitter)
        : program(program),
          emitter(emitter) {}

    pTypeNode visit(AutoNode&) override;
    pTypeNode visit(GenericNode&) override;
    pTypeNode visit(UnionNode&) override;
    pTypeNode visit(FunctionTypeNode&) override;
    pTypeNode visit(AggregateNode&) override;

private:
    ProgramData&              program;
    [[maybe_unused]] Emitter& emitter;
};

class TypeVisitor : public NodeVisitor {
public:
    TypeVisitor(ProgramData& program, Emitter& emitter)
        : program(program),
          emitter(emitter) {}

    void visit(DeclarationNode&) override;
    void visit(AssignNode&) override;
    void visit(FunctionNode&) override;

private:
    ProgramData& program;
    Emitter&     emitter;
};

class StmtVisitor : public NodeVisitor {
public:
    StmtVisitor(ProgramData& program, Emitter& emitter, RegisterAllocator& allocator)
        : program(program),
          emitter(emitter),
          allocator(allocator),
          expression_visitor(program, emitter, allocator),
          decay_visitor(program, emitter),
          type_visitor(program, emitter) {}

    void visit(DeclarationNode&) override;
    void visit(ScopeNode&) override;
    void visit(FunctionNode&) override;
    void visit(AssignNode&) override;
    void visit(IfNode&) override;
    void visit(WhileNode&) override;
    void visit(ExprStmtNode&) override;

    inline bool failed() override {
        return visitor_failed || expression_visitor.failed() || decay_visitor.failed() ||
               type_visitor.failed();
    }

private:
    ProgramData&       program;
    Emitter&           emitter;
    RegisterAllocator& allocator;

    ExprVisitor  expression_visitor;
    DecayVisitor decay_visitor;
    TypeVisitor  type_visitor;

    size_t label_counter = 0;

    std::optional<size_t> escape_label = std::nullopt;
    std::optional<size_t> repeat_label = std::nullopt;
};

class PrintVisitor : public NodeVisitor {
public:
    void visit(LiteralNode&, Operand) override;
    void visit(SymbolNode&, Operand) override;
    void visit(UnaryNode&, Operand) override;
    void visit(GroupNode&, Operand) override;
    void visit(BinaryNode&, Operand) override;
};

VIA_NAMESPACE_END

#endif
