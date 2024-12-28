from utils import log_message
import os
import sys

if len(sys.argv) != 2:
    log_message('Usage: python3 count_lines.py <directory_path>')
    sys.exit(1)

directory = sys.argv[1]
possible_submodules = ['luau',]
accepted_extensions = ('.cpp', '.h')

def get_lines_in_directory(file_path: str):
    with open(file_path, 'r') as f:
        return f.readlines()

def main():
    log_message('Getting lines of code in directory')
    total = 0
    for dirpath, dirnames, filenames in os.walk(directory):
        # Skip submodules by name
        if any(submodule in dirpath for submodule in possible_submodules):
            print(f'Skipping submodule: {dirpath}')
            continue
        
        # Only count lines for files, not directories
        for filename in filenames:
            file_path = os.path.join(dirpath, filename)
            try:
                if file_path.endswith(accepted_extensions):
                    lines = get_lines_in_directory(file_path)
                    total += len(lines)
                    print(f'Directory: {dirpath}, File: {filename}, Lines: {len(lines)}')
            except Exception as e:
                print(f'Error reading file {file_path}: {e}')
                
    print(f'Total Lines: {total}')

if __name__ == '__main__':
    main()
