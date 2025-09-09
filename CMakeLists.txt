cmake_minimum_required(VERSION 3.20)
project(via LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-fdiagnostics-color=always)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/diagnostics:color)
endif()

# Dependencies
add_subdirectory(thirdparty/spdlog)
add_subdirectory(thirdparty/magic_enum)
add_subdirectory(thirdparty/argparse)
add_subdirectory(thirdparty/mimalloc)
add_subdirectory(thirdparty/cpptrace)
add_subdirectory(thirdparty/csv2)

# Modules
add_subdirectory(src/via-core)
add_subdirectory(src/via-cli)
add_subdirectory(src/via-lib)
