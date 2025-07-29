add_library(via-shared INTERFACE)

target_include_directories(via-shared INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(via::shared ALIAS via-shared)
