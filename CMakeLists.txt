cmake_minimum_required(VERSION 3.27)
project(via LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(
        -fdiagnostics-color=always
        -Wextra
        -Wall
        # Disable useless warnings
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-unused-but-set-variable
        $<$<CONFIG:Debug>:-fsanitize=address,undefined,leak>
    )

    add_link_options(
        $<$<CONFIG:Debug>:-fsanitize=address,undefined,leak>
    )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/diagnostics:color)
endif()

# Dependencies
find_package(spdlog CONFIG REQUIRED)
find_package(CLI11 CONFIG REQUIRED)
find_package(mimalloc CONFIG REQUIRED)
find_package(cpptrace CONFIG REQUIRED)
find_package(replxx CONFIG REQUIRED)

# Modules
add_subdirectory(src/via-core)
add_subdirectory(src/via-cli)
add_subdirectory(src/via-lib)
