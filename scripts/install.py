#!/usr/bin/env python3
# This file is a part of the via Programming Language project
# Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

import os
import sys
import platform
import shutil
from pathlib import Path

GITHUB_REPO="https://github.com/XnLogicaL/via-lang"

def get_core_path() -> Path:
  """
  Return the default core language path depending on OS conventions.
  Prefers user-local install paths.
  """
  system = platform.system()

  if system == "Windows":
    # Prefer LOCALAPPDATA, fallback to APPDATA, then user profile
    base = os.getenv("LOCALAPPDATA") or os.getenv("APPDATA") or str(Path.home())
    core = Path(base) / "via"

  elif system == "Darwin":  # macOS
    core = Path.home() / "Library" / "Application Support" / "via"

  elif system == "Linux":
    # Follow XDG spec if available
    xdg = os.getenv("XDG_DATA_HOME")
    if xdg:
        core = Path(xdg) / "via"
    else:
        core = Path.home() / ".local" / "share" / "via"

  else:
    # Unknown OS → fallback to home dir
    core = Path.home() / ".via"

  return core

def create_dirs():
  core_path=get_core_path()
  core_path.mkdir(exist_ok=True)
  (core_path/"lib").mkdir(exist_ok=True)
  (core_path/"lib"/"std").mkdir(exist_ok=True)

def install_libs():
  core_path=get_core_path()
  shutil.copytree(Path(os.path.realpath(__file__))/".."/"lib"/"std"/"bin", core_path/"lib"/"std")

if __name__ == "__main__":
  print("Installing...")
  print(f"(): {get_core_path()=}")

  create_dirs()
  install_libs()
  
  print("Done")
  