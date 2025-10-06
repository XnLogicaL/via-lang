cmake_minimum_required(VERSION 3.20)

function(load_unit NAME)
    set(LIB_OUTPUT_DIR ${CMAKE_BINARY_DIR}/src/via-lib/${NAME})

    file(MAKE_DIRECTORY ${LIB_OUTPUT_DIR})
    file(GLOB STD_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${NAME}/*.cpp")

    foreach(src ${STD_SOURCES})
        get_filename_component(src_name ${src} NAME_WE)
        add_library(via-${src_name} SHARED ${src})

        target_link_libraries(via-${src_name} PRIVATE via::core)
        set_target_properties(via-${src_name}
            PROPERTIES
                OUTPUT_NAME ${src_name}         # just "foo.so" instead of "libfoo.so"
                PREFIX ""
                LIBRARY_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR}
                RUNTIME_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR}
        )

        add_custom_target(via-${src_name}_build ALL
            DEPENDS via-${src_name}
        )
    endforeach()
endfunction()

load_unit(std)
