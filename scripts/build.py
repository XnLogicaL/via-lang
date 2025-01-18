import os
from utils import log_message
from utils import run_command

BUILD_DIR = "./build"

def make_build_folder():
    os.mkdir(BUILD_DIR)

def prepare_build_folder():
    log_message("Preparing build folder [1/3]")
    # Check for old build folder
    if os.path.exists(BUILD_DIR):
        # Ask the user if they'd like to replace the folder
        decision = input("Build directory already exists, would you like to replace it? [y/n] ")
        if decision.lower() == "y":
            # Recursively wipe directory
            for root, dirs, files in os.walk(BUILD_DIR, topdown=False):
                for file in files:
                    # Remove files
                    os.remove(os.path.join(root, file))
                for dir in dirs:
                    # Remove subdirectories
                    os.rmdir(os.path.join(root, dir))
            # Remove the root directory
            os.rmdir(BUILD_DIR)
        else:
            log_message("Aborting build")
            exit(0)
    
    # Create new build directory
    make_build_folder()
    # Set current directory to the build directory
    os.chdir(BUILD_DIR)

def run_cmake():
    log_message("Running CMake [2/3]")
    run_command("cmake ..")

def run_make():
    log_message("Running Make [3/3]")
    run_command("make -j$(nproc)")

def main():
    log_message("Initializing build")
    prepare_build_folder()
    run_cmake()
    run_make()
    log_message("Build complete")

if __name__ == '__main__':
    main()