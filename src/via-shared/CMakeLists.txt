file(GLOB_RECURSE VIA_SHARED_SOURCES CONFIGURE_DEPENDS *.cpp)

add_library(via-shared STATIC ${VIA_SHARED_SOURCES})

target_include_directories(via-shared
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(via-shared
  PUBLIC  
    mimalloc
    spdlog
    magic_enum
    fmt::fmt
)

add_library(via::shared ALIAS via-shared)
