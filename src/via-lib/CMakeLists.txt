cmake_minimum_required(VERSION 3.20)

set(LIB_OUTPUT_DIR ${CMAKE_BINARY_DIR}/src/via-lib/std)
file(MAKE_DIRECTORY ${LIB_OUTPUT_DIR})
file(GLOB STD_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/std/*.cpp")

foreach(src ${STD_SOURCES})
    get_filename_component(src_name ${src} NAME_WE)
    add_library(${src_name} SHARED ${src})
    target_link_libraries(${src_name} PRIVATE via::core)
    set_target_properties(${src_name} PROPERTIES
        OUTPUT_NAME ${src_name}         # just "foo.so" instead of "libfoo.so"
        PREFIX ""
        LIBRARY_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR}
        RUNTIME_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR}
    )

    add_custom_target(${src_name}_build ALL
        DEPENDS ${src_name}
    )
endforeach()
