#!/usr/bin/env python3
# ===================================================== #
#  This file is a part of the via Programming Language  #
# ----------------------------------------------------- #
#           Copyright (C) XnLogicaL 2024-2025           #
#              Licensed under GNU GPLv3.0               #
# ----------------------------------------------------- #
#         https://github.com/XnLogicaL/via-lang         #
# ===================================================== #

import os
import subprocess
import sys

from utils import command, error, info


def format_recursive(directory):
    if not os.path.isdir(directory):
        error(f"no such file or directory: '{directory}'")
        sys.exit(1)

    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith((".cpp", ".hpp")):
                file_path = os.path.join(root, file)
                try:
                    command(f"clang-format -i {file_path}")
                    info(f"formatted: {file_path}")
                except subprocess.CalledProcessError as e:
                    error(f"error formatting file: {file_path}")
                    print(f" |-> {e}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        info("usage: format.py <directory>")
        exit(1)

    directory = sys.argv[1]
    format_recursive(directory)
