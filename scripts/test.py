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

from utils import error, info, warn


def test(binary, file):
    result = subprocess.run([binary, "run", file], capture_output=True, text=True)
    output = result.stdout.strip()

    with open(f"{file}.out", "r") as f:
        expected = f.read().strip()

    if output == expected:
        info(f"test passed: {file}")
    else:
        warn(f"test failed: {file}")
        print(f" |-> Expected: {expected}")
        print(" |")
        print(f" |-> stdout: {output}")
        print(f" |-> stderr: {result.stderr.strip()}")
        return False
    return True


def run(binary, tests):
    passed = True

    for test_file in os.listdir(tests):
        if test_file.endswith(".via"):
            test_file_path = os.path.join(tests, test_file)
            if not test(binary, test_file_path):
                passed = False

    return passed


if __name__ == "__main__":
    if len(sys.argv) < 3:
        info("usage: test.py <binary> <dir>")
        exit(1)

    if run(sys.argv[1], sys.argv[2]):
        info("all tests passed")
    else:
        error("some tests failed")
        exit(1)
