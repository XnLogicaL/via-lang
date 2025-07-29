cmake_minimum_required(VERSION 3.20)
project(via LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
add_subdirectory(thirdparty/spdlog)
add_subdirectory(thirdparty/magic_enum)
add_subdirectory(thirdparty/argparse)
add_subdirectory(thirdparty/mimalloc)
add_subdirectory(thirdparty/fmt)

# Modules
add_subdirectory(src/via-shared)
add_subdirectory(src/via-core)
add_subdirectory(src/via-cli)
