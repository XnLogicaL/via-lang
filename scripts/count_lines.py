# This file is a part of the via Programming Language project
# Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

from utils import log_message
import os
import sys

if len(sys.argv) != 2:
    log_message('Usage: python3 count_lines.py <directory_path>')
    sys.exit(1)

DIR = sys.argv[1]
SUBDIR_WHITELIST = ['src', 'include', 'test', 'fuzz', 'CLI',]
EXT_WHITELIST = ('.c', '.cpp', '.h', '.hpp')

def get_lines_in_file(file_path: str) -> int:
    """
    Reads the file and returns the number of lines.
    """
    with open(file_path, 'r', encoding='utf-8') as f:
        return sum(1 for _ in f)

def is_whitelisted_directory(dirpath: str) -> bool:
    """
    Check if the directory path starts with one of the whitelisted subdirectories.
    """
    rel_path = os.path.relpath(dirpath, DIR)
    if rel_path == ".":
        return True  # Root directory should always be included
    first_component = rel_path.split(os.sep)[0]
    return first_component in SUBDIR_WHITELIST

def main():
    log_message(f"Counting lines of code in directory '{DIR}'")
    total_lines = 0

    # Traverse the directory
    for dirpath, _, filenames in os.walk(DIR):
        # Skip directories that are not in the whitelist
        if not is_whitelisted_directory(dirpath):
            continue
        
        # Process files in the current directory
        for filename in filenames:
            file_path = os.path.join(dirpath, filename)
            _, ext = os.path.splitext(filename)
            if ext in EXT_WHITELIST:
                try:
                    lines = get_lines_in_file(file_path)
                    total_lines += lines
                    print(f' File: {dirpath}/{filename}, Lines: {lines}')
                except Exception as err:
                    print(f' Error reading file {file_path}: {err}')

    log_message(f'Total Lines: {total_lines}')

if __name__ == '__main__':
    if not os.path.isdir(DIR):
        log_message(f'Error: {DIR} is not a valid directory.')
        sys.exit(1)
    main()
