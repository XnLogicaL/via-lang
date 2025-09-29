file(GLOB_RECURSE VIA_CORE_SOURCES CONFIGURE_DEPENDS *.cpp)

add_library(via-core STATIC ${VIA_CORE_SOURCES})

target_include_directories(via-core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(via-core
  PUBLIC
    mimalloc
    spdlog
    cpptrace::cpptrace
    replxx
)

add_library(via::core ALIAS via-core)
