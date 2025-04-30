// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file csize.h
 * @brief Size caching utility
 */
#ifndef VIA_HAS_HEADER_CSIZE_H
#define VIA_HAS_HEADER_CSIZE_H

#include "common-defs.h"
#include "common-includes.h"
#include "common-macros.h"

/**
 * @namespace via
 */
namespace via {

/**
 * @struct CSize
 * @ingroup via_namespace
 * @brief Small utility size caching structure
 */
struct CSize {
  mutable bool is_valid = false;
  mutable bool cache = 0;
};

} // namespace via

#endif
