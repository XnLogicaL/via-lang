# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(MyProject VERSION 1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define the source and header directories
set(SOURCE_DIR ./src)
set(INCLUDE_DIR ./include)

# Include all source files in the 'src' directory
file(GLOB_RECURSE SOURCES ${SOURCE_DIR}/*.cpp)

# Include all header files (directories) in the 'include' directory
file(GLOB_RECURSE INCLUDES ${INCLUDE_DIR}/*)

# Set the path to the LLVM build directory
set(LLVM_DIR ./llvm/build/lib/cmake/llvm)

# Find LLVM package (make sure to provide the correct path)
find_package(LLVM REQUIRED CONFIG PATHS ${LLVM_DIR})

# Specify the executable target for the main build
add_executable(main ${SOURCES})

# Include the 'include' directories for headers
target_include_directories(main PRIVATE ${INCLUDE_DIR})

# Link LLVM libraries to the 'main' executable
target_link_libraries(main PRIVATE LLVM::LLVM)

# Apply compilation flags (you can replace these with your actual flags)
target_compile_options(main PRIVATE
    -O3
    -Wall
    -Wextra
    -g
)

# Optional: Link other libraries here if needed
# target_link_libraries(main PRIVATE SomeLibrary)

# Specify the test build (for the 'vm' target in Makefile)
add_executable(test
    ./src/viavm.cpp
    ./src/Lexer/lexer.cpp
    ./src/Lexer/token.cpp
    ${SOURCE_DIR}/VM/*.cpp
    ${SOURCE_DIR}/Utils/*.cpp
)

# Apply the same compilation flags for the test executable
target_compile_options(test PRIVATE
    -O3
    -Wall
    -Wextra
    -g
)

# Include directories for the test executable
target_include_directories(test PRIVATE ${INCLUDE_DIR})

# Link LLVM libraries to the 'test' executable
target_link_libraries(test PRIVATE LLVM::LLVM)

# Declare the clean command (equivalent to `make clean`)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)

# Optional: Add testing functionality if desired (e.g., using CTest)
enable_testing()

# For cleaning up (just like 'make clean')
add_custom_target(clean COMMAND ${CMAKE_BUILD_TOOL} clean)

# Optionally, you can include other submodules or libraries here
# find_package(LLVM REQUIRED CONFIG)
# target_link_libraries(main PRIVATE LLVM::LLVM)
