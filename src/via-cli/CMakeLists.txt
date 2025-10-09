file(GLOB_RECURSE VIA_CLI_SOURCES CONFIGURE_DEPENDS *.cpp)

add_executable(via-cli ${VIA_CLI_SOURCES})

target_include_directories(via-cli
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ${mimalloc_INCLUDE_DIRS}
)

target_link_libraries(via-cli
    PUBLIC
        via::core
        CLI11::CLI11
        spdlog::spdlog
        ${mimalloc_LIBRARIES}
)

add_executable(via::cli ALIAS via-cli)
