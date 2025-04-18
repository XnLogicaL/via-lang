// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "constant.h"

// ================================================================ |
// constant.cpp
//
namespace via {

using constant_type = ConstantHolder::constant_type;
using constant_vector = ConstantHolder::constant_vector;

size_t ConstantHolder::size() const {
  return constants.size();
}

operand_t ConstantHolder::push_constant(constant_type constant) {
  for (size_t index = 0; index < constants.size(); index++) {
    const Value& val = constants[index];
    if VIA_UNLIKELY (val.compare(constant)) {
      return index;
    }
  }

  constants.emplace_back(constant.clone());
  return constants.size() - 1;
}

constant_type& ConstantHolder::at(size_t index) const {
  return constants.at(index);
}

constant_type& ConstantHolder::at_s(size_t index) const {
  static const Value Nil;
  if (index >= size()) {
    return Nil;
  }

  return at(index);
}

const constant_vector& ConstantHolder::get() const {
  return constants;
}

} // namespace via
