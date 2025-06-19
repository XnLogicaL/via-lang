# ----------------------------------------------------------------------------------------------------
# SHARED CONFIG
# ----------------------------------------------------------------------------------------------------

# Enforce minimum required generator version
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)

# Add dependencies
add_subdirectory(thirdparty/ArenaAllocator)
add_subdirectory(thirdparty/argparse)
add_subdirectory(thirdparty/mimalloc)

# Debug option
option(DEBUG_MODE "Enabled debug build mode" OFF)

# ----------------------------------------------------------------------------------------------------
# LIBRARY CONFIG
# ----------------------------------------------------------------------------------------------------

# Declare project
project(libvia CXX)

# Fetch sources
file(GLOB_RECURSE SOURCES src/*.cpp)

# Declare library
add_library(libvia STATIC ${SOURCES})

# Add dependencies
target_link_libraries(libvia PUBLIC libmimalloc)
target_link_libraries(libvia PUBLIC ArenaAllocator)

# Include dirs
target_include_directories(libvia include)

# Set build type based on DEBUG_MODE
if (DEBUG_MODE)
  set(CMAKE_BUILD_TYPE Debug)
else()
  set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler options
target_compile_options(libvia PUBLIC
  $<$<AND:$<CONFIG:Release>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-Ofast -march=native -flto>
  $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/Ox>
)

# Linker options
target_link_options(libvia PUBLIC
  $<$<AND:$<CONFIG:Release>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-flto>
)

# ----------------------------------------------------------------------------------------------------
# STANDALONE CONFIG
# ----------------------------------------------------------------------------------------------------

# Declare project
project(via CXX)

# Fetch sources
file(GLOB_RECURSE CLI_SOURCES CLI/*.cpp)

# Declare executable
add_executable(via STATIC "CLI/main.cpp" ${CLI_SOURCES})

# Add dependencies
target_link_libraries(via PUBLIC argparse)
target_link_libraries(via PUBLIC libvia)

# Include dirs
target_include_directories(via include)

