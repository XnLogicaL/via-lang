# ----------------------------------------------------------------------------------------------------
# SHARED CONFIG
# ----------------------------------------------------------------------------------------------------

# Enforce minimum required generator version
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)

# Convenient for LSPs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Debug option
option(DEBUG_MODE "Enable debug build" OFF)

# Set build type based on DEBUG_MODE
if (DEBUG_MODE)
  set(CMAKE_BUILD_TYPE Debug)
else()
  set(CMAKE_BUILD_TYPE Release)
endif()

# mimalloc stuff
set(MI_OVERRIDE OFF)

# Add dependencies
add_subdirectory(thirdparty/argparse)
add_subdirectory(thirdparty/mimalloc)
add_subdirectory(thirdparty/spdlog)
add_subdirectory(thirdparty/magic_enum)

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
target_link_libraries(libvia PUBLIC mimalloc)
target_link_libraries(libvia PUBLIC spdlog)
target_link_libraries(libvia PUBLIC magic_enum)

# Include dirs
target_include_directories(libvia PUBLIC include src)

# Compiler options
target_compile_options(libvia PUBLIC
  $<$<AND:$<CONFIG:Release>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-Ofast -march=native -flto>
  $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/Ox>
  # $<$<AND:$<CONFIG:Debug>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-fsanitize=address,undefined>
)

# Linker options
target_link_options(libvia PUBLIC
  $<$<AND:$<CONFIG:Release>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-flto>
  # $<$<AND:$<CONFIG:Debug>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-fsanitize=address,undefined>
)

# ----------------------------------------------------------------------------------------------------
# STANDALONE CONFIG
# ----------------------------------------------------------------------------------------------------

# Declare project
project(via CXX)

# Fetch sources
file(GLOB_RECURSE CLI_SOURCES CLI/*.cpp)

# Declare executable
add_executable(via "CLI/main.cpp" ${CLI_SOURCES})

# Add dependencies
target_link_libraries(via PUBLIC argparse)
target_link_libraries(via PUBLIC spdlog)
target_link_libraries(via PUBLIC magic_enum)
target_link_libraries(via PUBLIC libvia)

# Include dirs
target_include_directories(via PUBLIC include)

