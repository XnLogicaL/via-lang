import sys
import subprocess
import pkg_resources
import pip

def log_message(message):
    print("(): %s" % (message))

def run_command(command):
    try:
        result = subprocess.run(command, check=True, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        return result.stdout.decode('utf-8'), result.stderr.decode('utf-8')
    except subprocess.CalledProcessError as e:
        log_message(f"Error while running command '{command}'")
        log_message(f"  stderr: {e.stderr.decode('utf-8')}")
        sys.exit(1)
        
def resolve_dependency(package):
    print("Resolving dependency '%s'" % package)
    try:
        pkg_resources.get_distribution(package)
        print("Dependency '%s' found" % package)
    except pkg_resources.DistributionNotFound:
        pip.main(['install', package])
        print("Dependency '%s' not found, installing..." % package)
    print("Resolved dependency '%s'" % package)
    