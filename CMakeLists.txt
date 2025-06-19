# ----------------------------------------------------------------------------------------------------
# SHARED CONFIG
# ----------------------------------------------------------------------------------------------------

# Enforce minimum required generator version
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)

# Add dependencies
add_subdirectory(thirdparty/ArenaAllocator)

# ----------------------------------------------------------------------------------------------------
# LIBRARY CONFIG
# ----------------------------------------------------------------------------------------------------

# Declare project
project(via)


