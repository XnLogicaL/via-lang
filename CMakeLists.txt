# Set minimum required CMake version
cmake_minimum_required(VERSION 3.20)

# Define project name and language
project(via LANGUAGES CXX)

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)

# Allow options to define DEBUG_MODE and MAIN_FILE_PATH, with defaults
option(DEBUG_MODE "Set the build mode (DEBUG/PROD)" "PROD")
option(MAIN_FILE_PATH "Path to the main file (for fuzzing)" CLI/main.cpp)

# Print the current configuration
message(STATUS "Building with DEBUG_MODE: ${DEBUG_MODE}")
message(STATUS "Using MAIN_FILE_PATH: ${MAIN_FILE_PATH}")

# Add subdirectories for components
add_subdirectory(asmjit)
add_subdirectory(SHA256)
add_subdirectory(argparse)

# Set the compiler
if(DEBUG_MODE STREQUAL "DEBUG")
    message(STATUS "Compiling with Clang in DEBUG mode.")
    set(CMAKE_CXX_COMPILER /usr/bin/clang++)
else()
    message(STATUS "Compiling in PROD mode.")
    set(CMAKE_CXX_COMPILER /usr/bin/g++)
endif()

# Find sources
file(GLOB_RECURSE MAIN_SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE SRC_INCLUDES LIST_DIRECTORIES true src/*)

# Define the executable (with default main file for fuzzing)
add_executable(via ${MAIN_FILE_PATH} ${MAIN_SOURCES})

# Include directories
target_include_directories(via PRIVATE
    include/
    src/
    src/lex
    src/parse
    src/compiler
    src/preprocessor
    src/interpreter
)

# Set compiler flags
target_compile_options(via PUBLIC
    -Wall                  # Enable all warnings
    -Wextra                # Enable extra warnings
    -Werror                # Treat warnings as errors
    -g                     # Enable debugging symbols
)

# Add fuzzing specific flags for DEBUG mode
if(DEBUG_MODE STREQUAL "DEBUG")
    target_compile_definitions(via PRIVATE -DVIA_DEBUG)

    if (MAIN_FILE_PATH STREQUAL "CLI/main.cpp")
        target_compile_options(via PRIVATE -fsanitize=address,undefined)
        target_link_libraries(via PRIVATE -fsanitize=address,undefined)
    else()
        target_compile_options(via PRIVATE -fsanitize=address,fuzzer,undefined)
        target_link_libraries(via PRIVATE -fsanitize=address,fuzzer,undefined)
    endif()
else()
    target_compile_options(via PUBLIC
        -flto
        -march=native
        -mtune=native
        -Ofast
        -ffast-math
        -funroll-loops
        -ftree-vectorize
        -fomit-frame-pointer
        -fmerge-all-constants
        -finline-functions
        -ftree-pre
        -fprefetch-loop-arrays
        -fno-unwind-tables
        -fno-asynchronous-unwind-tables
        -falign-functions=32
        -falign-jumps=32
        -falign-loops=32
        -funsafe-math-optimizations
        -fno-stack-protector
        -fipa-pta
        -fdevirtualize-at-ltrans
        -fgcse-lm
        -fmerge-all-constants
        -fno-math-errno
    )
endif()

# Apply include directories for each subdirectory in src
foreach(dir ${SRC_INCLUDES})
    if(IS_DIRECTORY ${dir})
        target_include_directories(via PUBLIC ${dir})
    endif()
endforeach()

# Link libraries for external dependencies
target_link_libraries(via PUBLIC asmjit sha256 argparse)

# Optional: Link standard backtrace library if necessary
target_link_libraries(via PUBLIC stdc++_libbacktrace)

# Handle external definitions and linker settings for fuzzing
if(DEBUG_MODE STREQUAL "DEBUG" AND MAIN_FILE_PATH MATCHES "fuzz/.*\.cpp")
    message(STATUS "Fuzzing build enabled")
    target_compile_definitions(via PRIVATE -DFUZZING_ENABLED)
    target_compile_options(via PRIVATE -fsanitize=address,fuzzer,undefined)
    target_link_libraries(via PRIVATE -fsanitize=address,fuzzer,undefined)
endif()
