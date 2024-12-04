# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++23 -O3 -Wall -Wextra -g -ftemplate-depth=2048

# Include directories
INCLUDE_DIRS = ./src ./include
INCLUDES = $(foreach dir, $(INCLUDE_DIRS), $(shell find $(dir) -type d -exec printf '-I%s ' {} \;))

# Common source files (shared by all executables)
COMMON_SOURCES = $(shell find ./src -name '*.cpp' ! -name 'via.cpp' ! -name 'viac.cpp' ! -name 'viavm.cpp')

# Executables and their main source files
TARGETS = via viac viavm
VIA_MAIN = ./src/via.cpp
VIAC_MAIN = ./src/viac.cpp
VIAVM_MAIN = ./src/viavm.cpp

# Declare phony targets
.PHONY: all clean

# Default target to build all executables
all: $(TARGETS)

# Build rules for each executable
via: $(VIA_MAIN) $(COMMON_SOURCES)
	@echo "Building via (main executable)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

viac: $(VIAC_MAIN) $(COMMON_SOURCES)
	@echo "Building viac (compiler)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

viavm: $(VIAVM_MAIN) $(COMMON_SOURCES)
	@echo "Building viavm (virtual machine)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Clean build artifacts
clean:
	@echo "Cleaning up..."
	rm -f $(TARGETS)
