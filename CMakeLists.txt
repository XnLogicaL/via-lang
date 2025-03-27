# Set minimum required CMake version
cmake_minimum_required(VERSION 3.20)

# Define project name and language
project(via LANGUAGES CXX)

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)

# Static linking
set(BUILD_SHARED_LIBS OFF)

# Allow options to define DEBUG_MODE and MAIN_FILE_PATH, with defaults
option(DEBUG_MODE "Set the build mode (DEBUG/PROD)" "PROD")
option(MAIN_FILE_PATH "Path to the main file (for fuzzing)" "CLI/main.cpp")

# Print the current configuration
message(STATUS "Building with DEBUG_MODE: ${DEBUG_MODE}")
message(STATUS "Using MAIN_FILE_PATH: ${MAIN_FILE_PATH}")

# Add subdirectories for components
add_subdirectory(SHA256)
add_subdirectory(argparse)
add_subdirectory(magic_enum)
add_subdirectory(cpp-linenoise)
add_subdirectory(expected)

# Set the compiler
if(DEBUG_MODE STREQUAL "DEBUG")
    message(STATUS "Compiling with clang++ in DEBUG mode.")
    set(CMAKE_CXX_COMPILER /usr/bin/clang++)
else()
    message(STATUS "Compiling with g++ in PROD mode.")
    set(CMAKE_CXX_COMPILER /usr/bin/g++)
    set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

# Find sources
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE INCLUDES LIST_DIRECTORIES true include/*.h)

# Define the executable (with default main file for fuzzing)
add_executable(via ${MAIN_FILE_PATH} ${SOURCES})

# Include directories
target_include_directories(via PUBLIC
    include/
    include/via
    include/via/lex
    include/via/parse
    include/via/compiler
    include/via/preprocessor
    include/via/interpreter
    include/via/utility
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
    target_compile_definitions(via PUBLIC -DVIA_DEBUG)

    if (MAIN_FILE_PATH STREQUAL "CLI/main.cpp")
        target_compile_options(via PUBLIC -fsanitize=address,undefined)
        target_link_libraries(via PUBLIC -fsanitize=address,undefined)
    else()
        target_compile_options(via PUBLIC -fsanitize=address,fuzzer,undefined)
        target_link_libraries(via PUBLIC -fsanitize=address,fuzzer,undefined)
    endif()
else()
    target_compile_options(via PUBLIC
        -flto=auto
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

# Link libraries for external dependencies
target_link_libraries(via PUBLIC sha256 argparse linenoise magic_enum expected)

# Link standard backtrace library
target_link_libraries(via PUBLIC stdc++_libbacktrace)

# Handle external definitions and linker settings for fuzzing
if(DEBUG_MODE STREQUAL "DEBUG" AND MAIN_FILE_PATH MATCHES "fuzz/.*\.cpp")
    message(STATUS "Fuzzing build enabled")
    target_compile_definitions(via PUBLIC -DFUZZING_ENABLED)
    target_compile_options(via PUBLIC -fsanitize=address,fuzzer,undefined)
    target_link_libraries(via PUBLIC -fsanitize=address,fuzzer,undefined)
endif()
