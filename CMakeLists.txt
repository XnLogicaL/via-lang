cmake_minimum_required(VERSION 3.27)
project(via LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    set(MI_SECURE $<$<CONFIG:Debug>:ON>)
    # set(MI_TRACK_ASAN $<$<CONFIG:Debug>:ON>)
    set(MI_DEBUG_FULL $<$<CONFIG:Debug>:ON>)

    add_compile_options(
        -fdiagnostics-color=always
        -Wextra
        -Wall
        # Disable useless warnings
        -Wno-unused-parameter
        -Wno-unused-variable
        -Wno-unused-but-set-variable
        # $<$<CONFIG:Debug>:-fsanitize=address,undefined,leak>
    )

    add_link_options(
        # $<$<CONFIG:Debug>:-fsanitize=address,undefined,leak>
    )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/diagnostics:color)
endif()

add_subdirectory(thirdparty/mimalloc)
add_subdirectory(thirdparty/cpptrace)

# Modules
add_subdirectory(core)
add_subdirectory(cli)
add_subdirectory(lib)
