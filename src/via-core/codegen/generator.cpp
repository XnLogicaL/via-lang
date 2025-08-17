// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "generator.h"

namespace via {

using sema::ConstValue;

Header Generator::generate() {
  gen::StmtVisitor visitor(*this);

  for (const ast::StmtNode* stmt : m_ast)
    stmt->accept(visitor);

  emit_instruction(Opcode::HALT);
  return m_header;
}

void Generator::emit_instruction(Opcode op, Array<u16, 3> ops) {
  m_header.bytecode.push_back({op, ops[0], ops[1], ops[2]});
}

void Generator::emit_constant(ConstValue&& cv, u16* kp) {
  const usize sz = m_header.consts.size();
  for (usize i = 0; i < sz; i++) {
    if (m_header.consts.at(i).compare(cv)) {
      *kp = i;
      return;
    }
  }

  *kp = sz;
  m_header.consts.push_back(std::move(cv));
}

}  // namespace via
