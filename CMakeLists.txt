# ----------------------------------------------------------------------------------------------------
# SHARED CONFIG
# ----------------------------------------------------------------------------------------------------

# Enforce minimum required generator version
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)

# Convenient for LSPs
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# mimalloc stuff
set(MI_OVERRIDE OFF)

# Add dependencies
add_subdirectory(thirdparty/argparse)
add_subdirectory(thirdparty/mimalloc)
add_subdirectory(thirdparty/spdlog)
add_subdirectory(thirdparty/magic_enum)

# ----------------------------------------------------------------------------------------------------
# CORE CONFIG
# ----------------------------------------------------------------------------------------------------

# Declare project
project(via-core CXX)

# Fetch core sources
file(GLOB_RECURSE SOURCES src/via-core/*.cpp)

# Declare library
add_library(via-core STATIC ${SOURCES})

# Public interface: only headers from `include/`
target_include_directories(via-core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/via-shared
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/via-core
)

# Link dependencies
target_link_libraries(via-core PUBLIC
  mimalloc
  spdlog
  magic_enum
)

# Compiler options
target_compile_options(via-core PUBLIC
  $<$<AND:$<CONFIG:Release>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-Ofast -march=native -flto>
  $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/Ox>
   #$<$<AND:$<CONFIG:Debug>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-fsanitize=address,undefined>
)

# Linker options
target_link_options(via-core PUBLIC
  $<$<AND:$<CONFIG:Release>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-flto>
  # $<$<AND:$<CONFIG:Debug>,$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>>:-fsanitize=address,undefined>
)

# ----------------------------------------------------------------------------------------------------
# CLI CONFIG
# ----------------------------------------------------------------------------------------------------

# Declare project
project(via-cli CXX)

# Fetch sources
file(GLOB_RECURSE CLI_SOURCES src/via-cli/*.cpp)

# Executable sources
add_executable(via-cli ${CLI_SOURCES})

# Public interface for cli should be minimal
target_include_directories(via-cli
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/via-core
)

# Link dependencies
target_link_libraries(via-cli PUBLIC
  argparse
  spdlog
  magic_enum
  via-core
)

add_library(via::core ALIAS via-core)
add_executable(via::cli ALIAS via-cli)
