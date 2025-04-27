# This file is a part of the via Programming Language project
# Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

import os
import subprocess
import sys
from utils import run_command

def format_files_in_directory(directory):
    # Check if the directory exists
    if not os.path.isdir(directory):
        print(f"Error: Directory '{directory}' does not exist.")
        sys.exit(1)

    # Iterate through files in the directory and its subdirectories
    for root, _, files in os.walk(directory):
        for file in files:
            # Target only specific file extensions
            if file.endswith(('.cpp', '.hpp', '.c', '.h')):
                file_path = os.path.join(root, file)
                try:
                    # Run clang-format on the file
                    run_command(f"clang-format -i {file_path}")
                    print(f"Formatted: {file_path}")
                except subprocess.CalledProcessError as e:
                    print(f"Error formatting file: {file_path}")
                    print(e)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python format_directory.py <directory>")
        sys.exit(1)

    target_directory = sys.argv[1]
    format_files_in_directory(target_directory)
