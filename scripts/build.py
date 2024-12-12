from utils import via_log_message
from utils import run_command
import os
import sys

# Check if spec name is passed as an argument
if len(sys.argv) < 2:
    via_log_message("Usage: python3 build.py <spec>")
    sys.exit(1)

def build_spec(spec: str):
    via_log_message("Building via specification: " + spec)
    stdout, stderr = run_command(f"make -j$(nproc) {spec}")
    if stderr:
        via_log_message("Error building via specification: " + stderr)
        sys.exit(1)

def main():
    spec_name = sys.argv[1]
    build_spec(spec_name)

if __name__ == '__main__':
    main()