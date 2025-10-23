file(GLOB_RECURSE VIA_CORE_SOURCES CONFIGURE_DEPENDS *.cpp)

add_library(via SHARED ${VIA_CORE_SOURCES})

target_include_directories(via
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
    PRIVATE
        ${mimalloc_INCLUDE_DIRS}
)

target_link_libraries(via
    PUBLIC
        replxx::replxx
        spdlog::spdlog
        cpptrace::cpptrace
    PRIVATE
        mimalloc
)

add_library(via::core ALIAS via)
