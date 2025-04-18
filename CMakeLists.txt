# Set minimum required CMake version
cmake_minimum_required(VERSION 3.20)

# Define project name and language
project(via LANGUAGES CXX)

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)

# Static linking
set(BUILD_SHARED_LIBS OFF)

# Allow options to define DEBUG_MODE with default
option(DEBUG_MODE "Set the build mode" OFF)

# Print current configuration
message(STATUS "Building with DEBUG_MODE: ${DEBUG_MODE}")

# Add subdirectories
add_subdirectory(SHA256)
add_subdirectory(argparse)
add_subdirectory(magic_enum)
add_subdirectory(cpp-linenoise)
add_subdirectory(expected)
add_subdirectory(ArenaAllocator)

# Detect compiler
if(MSVC)
  set(PROD_COMPILER "MSVC")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(PROD_COMPILER "CLANG")
else()
  set(PROD_COMPILER "GCC")
endif()

# Compiler switch for production builds
if(DEBUG_MODE)
  message(STATUS "Compiling in DEBUG mode")
else()
  message(STATUS "Compiling with ${PROD_COMPILER} in PROD mode")

  if(PROD_COMPILER STREQUAL "GCC")
    set(CMAKE_CXX_COMPILER /usr/bin/g++)
  elseif(PROD_COMPILER STREQUAL "CLANG")
    set(CMAKE_CXX_COMPILER /usr/bin/clang++)
  endif()

  # Enable static linkage
  set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE HEADERS "include/*.hpp" "include/*.h" "src/*.hpp" "src/*.h")

# Define executable
add_executable(via "CLI/main.cpp" ${SOURCES})

# Include dirs
target_include_directories(via PUBLIC
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/include/via
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/compiler
  ${CMAKE_SOURCE_DIR}/src/interpreter
  ${CMAKE_SOURCE_DIR}/src/lex
  ${CMAKE_SOURCE_DIR}/src/parse
  ${CMAKE_SOURCE_DIR}/src/preprocessor
  ${CMAKE_SOURCE_DIR}/src/utility
)

# Global warnings and standards
if (NOT MSVC)
  target_compile_options(via PRIVATE
    -Wall
    -Wextra
    -Werror
  )
endif()

# Compiler-specific flags
if(DEBUG_MODE)
  if (NOT MSVC)
    target_compile_options(via PRIVATE -Og -g -fsanitize=address,undefined -fno-elide-constructors)
    target_link_options(via PRIVATE -fsanitize=address,undefined)
  endif()
else()
  if(NOT MSVC)
    target_compile_options(via PRIVATE
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
endif()

# Link dependencies
target_link_libraries(via PUBLIC sha256 argparse linenoise magic_enum expected ArenaAllocator)

# Backtrace support (non-MSVC only)
if (NOT MSVC)
  target_link_libraries(via PUBLIC stdc++_libbacktrace)
endif()
