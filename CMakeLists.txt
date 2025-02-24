# Set minimum required CMake version to 3.20.0
cmake_minimum_required(VERSION 3.20.0)

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)

# Constants
set(DEFAULT_DEBUG_MODE "PROD")
set(DEFAULT_MAIN_FILE_PATH CLI/main.cpp)

# Define project
project(via)

# Generate sources
file(GLOB_RECURSE MAIN_SOURCES CONFIGURE_DEPENDS src/*.cpp) # Automatically include all .cpp files in src
file(GLOB_RECURSE SRC_INCLUDES LIST_DIRECTORIES true src/*) # Recursively get all directories in the src folder

# Check for debug level
if(NOT DEFINED DEBUG_MODE)
    message(WARNING "DEBUG_MODE is not defined, using default: '${DEFAULT_DEBUG_MODE}'")
    set(DEBUG_MODE DEFAULT_DEBUG_MODE CACHE STRING "Debug level (e.g., 'PROD' or 'DEBUG')")
endif()

if(DEBUG_MODE STREQUAL "DEBUG")
    set(CMAKE_CXX_COMPILER /usr/bin/clang++)
endif()

# Check for main file path
if(NOT DEFINED MAIN_FILE_PATH)
    message(WARNING "MAIN_FILE_PATH is not defined, using default: '${DEFAULT_MAIN_FILE_PATH}'")
    set(MAIN_FILE_PATH "${DEFAULT_MAIN_FILE_PATH}")
else()
    if(NOT MAIN_FILE_PATH STREQUAL DEFAULT_MAIN_FILE_PATH AND DEBUG_MODE STREQUAL "PROD")
        message(FATAL_ERROR "MAIN_FILE_PATH must be '${DEFAULT_MAIN_FILE_PATH}' in production mode; otherwise symbol `main` may not be defined")
    endif()
endif()

# Create the executable
add_executable(via "${MAIN_FILE_PATH}" ${MAIN_SOURCES})

# Set compiler flags
target_compile_options(via PUBLIC
    -Wall                   # Enable all warnings
    -Wextra                 # Enable extra warnings
    -Werror                 # Treat warnings as errors
    -g                      # Enable debugging symbols
    -O3                     # Optimization level 3
)

# Set DEBUG flags if applicable
if(DEBUG_MODE STREQUAL "DEBUG")
    target_compile_definitions(via PRIVATE -DVIA_DEBUG)
    target_compile_options(via PRIVATE -fsanitize=address,fuzzer,undefined)
    target_link_options(via PRIVATE -fsanitize=address,fuzzer,undefined)
endif()

# Add subdirectories for different components
add_subdirectory(asmjit)
add_subdirectory(SHA256)
add_subdirectory(ArenaAllocator)

# Include directories
target_include_directories(via PUBLIC include/)
target_include_directories(via PUBLIC src/)

# Apply include directories for each subdirectory in src
foreach(dir ${SRC_INCLUDES})
    if(IS_DIRECTORY ${dir})
        target_include_directories(via PUBLIC ${dir})
    endif()
endforeach()

# Link libraries
target_link_libraries(via PUBLIC asmjit)
target_link_libraries(via PUBLIC sha256)
target_link_libraries(via PUBLIC ArenaAllocator)
target_link_libraries(via PUBLIC stdc++_libbacktrace)  # For std::backtrace
