# Set minimum required CMake version to 3.20.0
cmake_minimum_required(VERSION "3.20.0")

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)

# Define project
project(via)

# Automatically include all .cpp files in src and CLI folders
file(GLOB_RECURSE MAIN_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE CLI_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/CLI/*.cpp")
# Recursively get all directories in the src folder
file(GLOB_RECURSE SRC_INCLUDES LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/src/*)
# Recursively get all directories in the CLI folder
file(GLOB_RECURSE CLI_INCLUDES LIST_DIRECTORIES true ${CMAKE_SOURCE_DIR}/CLI/*)

# Create the executable
add_executable(via "${CMAKE_SOURCE_DIR}/CLI/main.cpp" ${MAIN_SOURCES} ${CLI_SOURCES})

# Set compiler flags
target_compile_options(via
    PRIVATE
    -Wall              # Enable all warnings
    -Wextra            # Enable extra warnings
    -g                 # Enable debugging symbols
    -O3                # Optimization level 3
)

# Add subdirectories for different components
add_subdirectory("AsmJit")

# Include directories
target_include_directories(via PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_include_directories(via PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/")

# Apply include directories for each subdirectory in src
foreach(dir ${SRC_INCLUDES})
    if(IS_DIRECTORY ${dir})
        target_include_directories(via PRIVATE ${dir})
    endif()
endforeach()

# Apply include directories for each subdirectory in CLI
foreach(dir ${CLI_INCLUDES})
    if(IS_DIRECTORY ${dir})
        target_include_directories(via PRIVATE ${dir})
    endif()
endforeach()

# Link libraries
target_link_libraries(via PUBLIC asmjit)
