/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "repair.hpp"
#include <iostream>
#include <spdlog/spdlog.h>

void via::RepairTool::scan_all()
{
    scan_core();
}

void via::RepairTool::log_category(const char* message)
{}

void via::RepairTool::log_check(const char* message)
{}

void via::RepairTool::log_result(bool ok)
{}

void via::RepairTool::scan_core()
{}
