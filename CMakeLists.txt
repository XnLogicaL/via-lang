# Set minimum required CMake version to 3.20.0
cmake_minimum_required(VERSION "3.20.0")

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)

# Define project
project(via)

# Automatically include all .cpp files in src and CLI folders
file(GLOB_RECURSE MAIN_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
# Recursively get all directories in the src folder
file(GLOB_RECURSE SRC_INCLUDES LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/src/*)

# Create the executable
if(DEFINED MAIN_FILE_PATH)
    add_executable(via ${MAIN_FILE_PATH} ${MAIN_SOURCES})
else()
    message(FATAL_ERROR "MAIN_FILE_PATH is not defined; cannot link main file\n  tip: use -DMAIN_FILE_PATH=...")
endif()

# Set compiler flags
target_compile_options(via
    PRIVATE
    -Wall                   # Enable all warnings
    -Wextra                 # Enable extra warnings
    -Werror
    -g                      # Enable debugging symbols
    -O3                     # Optimization level 3
    -fsanitize=address,fuzzer,undefined
)

target_link_options(via
    PRIVATE
    -fsanitize=address,fuzzer,undefined
)

# Add subdirectories for different components
add_subdirectory(asmjit)
add_subdirectory(SHA256)
add_subdirectory(ArenaAllocator)

# Include directories
target_include_directories(via PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(via PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/")

# Apply include directories for each subdirectory in src
foreach(dir ${SRC_INCLUDES})
    if(IS_DIRECTORY ${dir})
        target_include_directories(via PRIVATE ${dir})
    endif()
endforeach()

# Link libraries
target_link_libraries(via PUBLIC asmjit)
target_link_libraries(via PUBLIC sha256)
target_link_libraries(via PUBLIC ArenaAllocator)
target_link_libraries(via PUBLIC stdc++_libbacktrace) # For std::backtrace
