/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.h>
#include <via/types.h>

namespace via
{

struct RelSourceLoc
{
  usize line;
  usize offset;
};

struct SourceLoc
{
  usize begin;
  usize end;

  // Returns the absolute location as a relative location.
  RelSourceLoc toRelative(const std::string& source) const;
};

}  // namespace via
