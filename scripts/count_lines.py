from utils import via_log_message
import os
import sys

if len(sys.argv) != 2:
    via_log_message('Usage: python3 count_lines.py <directory_path>')
    sys.exit(1)

directory = sys.argv[1]
possible_submodules = ['luau', 'doctest']
accepted_extensions = ('.cpp', '.h')

def get_lines_in_directory(file_path: str):
    with open(file_path, 'r') as f:
        return f.readlines()

def main():
    via_log_message('Getting lines of code in directory')
    total = 0
    for dirpath, dirnames, filenames in os.walk(directory):
        # Skip submodules by name
        if any(submodule in dirpath for submodule in possible_submodules):
            via_log_message(f'Skipping submodule: {dirpath}')
            continue
        
        # Only count lines for files, not directories
        for filename in filenames:
            file_path = os.path.join(dirpath, filename)
            try:
                if file_path.endswith(accepted_extensions):
                    lines = get_lines_in_directory(file_path)
                    total += len(lines)
                    via_log_message(f'Directory: {dirpath}, File: {filename}, Lines: {len(lines)}')
            except Exception as e:
                via_log_message(f'Error reading file {file_path}: {e}')
                
    via_log_message(f'Total Lines: {total}')

if __name__ == '__main__':
    main()
