from utils import via_log_message
from utils import run_command
import os
import sys

# Check if executable name is passed as an argument
if len(sys.argv) < 2:
    via_log_message("Usage: python3 profile.py <executable_name>")
    sys.exit(1)

executable = sys.argv[1]
callgrind_output = 'callgrind.out'

# Run the executable with Callgrind (Valgrind)
def run_with_callgrind():
    via_log_message(f"Running {executable} with Callgrind...")
    command = f'valgrind --tool=callgrind --callgrind-out-file={callgrind_output} {executable}'
    for i in range(2, len(sys.argv)):
        command += f' {sys.argv[i]}'
    stdout, stderr = run_command(command)
    via_log_message(f"Standard Output:\n{stdout}")
    if stderr:
        via_log_message(f"Error during execution:\n{stderr}")

# Analyze Callgrind output
def analyze_callgrind_output():
    via_log_message(f"Analyzing Callgrind output: {callgrind_output}")
    if os.path.exists(callgrind_output):
        stdout, stderr = run_command(f'kcachegrind {callgrind_output}')
        if stderr:
            via_log_message(f"Error during execution:\n{stderr}")
        run_command(f'rm -rf {callgrind_output}')
    else:
        via_log_message("No Callgrind output found.")

# Optional: Build project before running (uncomment if needed)
def build_project():
    via_log_message("Building project before running the executable...")
    build_command = 'make'  # Assuming make is used to build the project
    run_command(build_command)

def main():
    # Uncomment if you want to ensure the project is built before profiling
    # build_project()
    
    run_with_callgrind()
    analyze_callgrind_output()

if __name__ == "__main__":
    main()