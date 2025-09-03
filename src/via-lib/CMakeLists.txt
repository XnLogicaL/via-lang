cmake_minimum_required(VERSION 3.20)

# Output directory for .so/.dll/.dylib
set(LIB_OUTPUT_DIR ${CMAKE_BINARY_DIR}/src/via-lib/std)
file(MAKE_DIRECTORY ${LIB_OUTPUT_DIR})

# All .cpp files in this folder
file(GLOB STD_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/std/*.cpp")

foreach(src ${STD_SOURCES})
  get_filename_component(src_name ${src} NAME_WE)

  # Shared library from this source
  add_library(${src_name} SHARED ${src})

  # Link to core (symbols available, not merged)
  target_link_libraries(${src_name} PRIVATE via::core)

  # Put it in build/src/via-lib/std
  set_target_properties(${src_name} PROPERTIES
    OUTPUT_NAME ${src_name}         # just "foo.so" instead of "libfoo.so"
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR}
  )

  # Ensure it’s built by default
  add_custom_target(${src_name}_build ALL
    DEPENDS ${src_name}
  )
endforeach()
