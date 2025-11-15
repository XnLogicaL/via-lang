/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

namespace via {

class RepairTool final
{
  public:
    void scan_all();

  private:
    void log_category(const char* message);
    void log_check(const char* message);
    void log_result(bool ok);

    void scan_core();

  private:
};

} // namespace via
