file(GLOB_RECURSE VIA_CORE_SOURCES CONFIGURE_DEPENDS *.cpp)

add_library(via-core SHARED ${VIA_CORE_SOURCES})

target_include_directories(via-core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${mimalloc_INCLUDE_DIRS}
)

target_link_libraries(via-core
    PUBLIC
        replxx::replxx
        spdlog::spdlog
        cpptrace::cpptrace
    PRIVATE
        mimalloc
)

add_library(via::core ALIAS via-core)
