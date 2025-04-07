//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#include "constant.h"

// ================================================================ |
// constant.cpp
//
namespace via {

using constant_type = constant_holder::constant_type;
using constant_vector = constant_holder::constant_vector;

size_t constant_holder::size() const {
  return constants.size();
}

operand_t constant_holder::push_constant(constant_type& constant) {
  for (size_t index = 0; index < constants.size(); index++) {
    const value_obj& val = constants[index];
    if VIA_UNLIKELY (val.compare(constant)) {
      return index;
    }
  }

  constants.emplace_back(constant.clone());
  return constants.size() - 1;
}

constant_type& constant_holder::at(size_t index) const {
  return constants.at(index);
}

constant_type& constant_holder::at_s(size_t index) const {
  static const value_obj nil;
  if (index >= size()) {
    return nil;
  }

  return at(index);
}

const constant_vector& constant_holder::get() const {
  return constants;
}

} // namespace via
