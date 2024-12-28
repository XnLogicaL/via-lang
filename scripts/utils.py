import sys
import subprocess

def log_message(message):
    print("[ via ]: %s" % (message))

def run_command(command):
    try:
        result = subprocess.run(command, check=True, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return result.stdout.decode('utf-8'), result.stderr.decode('utf-8')
    except subprocess.CalledProcessError as e:
        log_message(f"Error while running command: {e}")
        log_message(f"Standard Error Output: {e.stderr.decode('utf-8')}")
        sys.exit(1)