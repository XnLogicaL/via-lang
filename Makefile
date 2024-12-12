# Compiler and flags
CXX = ccache g++
CXXFLAGS = -std=c++23 -O3 -Wall -Wextra -g

# Include directories
INCLUDE_DIRS = ./src ./include
INCLUDES = $(foreach dir, $(INCLUDE_DIRS), $(shell find $(dir) -type d -exec printf '-I%s ' {} \;))

# Build directory
BUILD_DIR = ./build

# Common source files (shared by all executables)
COMMON_SOURCES = $(shell find ./src -name '*.cpp' ! -name 'via.cpp' ! -name 'viac.cpp' ! -name 'viavm.cpp')

# Executables and their main source files
TARGETS = via viac viavm
VIA_MAIN = ./src/via.cpp
VIAC_MAIN = ./src/viac.cpp
VIAVM_MAIN = ./src/viavm.cpp

# Declare phony targets
.PHONY: all clean asm

# Default target to build all executables
all: $(TARGETS)

# Build rules for each executable
via: $(BUILD_DIR)/via.o $(COMMON_SOURCES:./src/%.cpp=$(BUILD_DIR)/%.o)
	@echo "Building via (main executable)..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $^

viac: $(BUILD_DIR)/viac.o $(COMMON_SOURCES:./src/%.cpp=$(BUILD_DIR)/%.o)
	@echo "Building viac (compiler)..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $^

viavm: $(BUILD_DIR)/viavm.o $(COMMON_SOURCES:./src/%.cpp=$(BUILD_DIR)/%.o)
	@echo "Building viavm (virtual machine)..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $^

# Generate object files in the build directory from source files
$(BUILD_DIR)/%.o: ./src/%.cpp
	@mkdir -p $(dir $@)  # Create the necessary directories for object files
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR) $(TARGETS)
