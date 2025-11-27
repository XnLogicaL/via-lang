#!/usr/bin/env python3
# ===================================================== #
#  This file is a part of the via Programming Language  #
# ----------------------------------------------------- #
#           Copyright (C) XnLogicaL 2024-2025           #
#              Licensed under GNU GPLv3.0               #
# ----------------------------------------------------- #
#         https://github.com/XnLogicaL/via-lang         #
# ===================================================== #

import subprocess
import sys


def info(message):
    print(f"[+] {message}")


def warn(message):
    print(f"[!] {message}")


def error(message):
    print(f"[X] {message}")


def command(cmd):
    try:
        result = subprocess.run(
            cmd,
            check=True,
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        return result.stdout.decode("utf-8"), result.stderr.decode("utf-8")
    except subprocess.CalledProcessError as e:
        error(f"Failed to run '{command}'")
        print(f" |-> stderr: {e.stderr.decode('utf-8')}")
        sys.exit(1)
