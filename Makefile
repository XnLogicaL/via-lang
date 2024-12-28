# Compiler and flags
CXX = ccache g++
CXXFLAGS = -std=c++23 -O3 -Wall -Wextra -g

# Include directories
INCLUDE_DIRS = ./src ./include ./CLI -I./AsmJit/src
INCLUDES = $(foreach dir, $(INCLUDE_DIRS), $(shell find $(dir) -type d -exec printf '-I%s ' {} \;))

# Build directory
BUILD_DIR = ./build

# Common source files (shared by all executables)
COMMON_SOURCES = $(shell find ./src -name '*.cpp')
COMMON_OBJECTS = $(COMMON_SOURCES:./src/%.cpp=$(BUILD_DIR)/%.o)

# CLI source files
CLI_SOURCES = $(shell find ./CLI -name '*.cpp')
CLI_OBJECTS = $(CLI_SOURCES:./CLI/%.cpp=$(BUILD_DIR)/CLI/%.o)

# Targets
TARGET = via

DEBUG_FLAGS = -DVIA_DEBUG=1

# Declare phony targets
.PHONY: all clean

# Default target to build the 'via' executable
all: $(PRELUDE) $(TARGET)

$(PRELUDE):
	@echo "Starting build..."

# Link the final executable
$(TARGET): $(COMMON_OBJECTS) $(CLI_OBJECTS)
	@echo "Linking object files..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $(COMMON_OBJECTS) $(CLI_OBJECTS) -o $(TARGET)
	@echo "Build complete"

# Generate object files for common sources (src)
$(BUILD_DIR)/%.o: ./src/%.cpp
	@mkdir -p $(dir $@)  # Ensure build subdirectories exist
	@echo "Compiling $< to $@..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Generate object files for CLI sources
$(BUILD_DIR)/CLI/%.o: ./CLI/%.cpp
	@mkdir -p $(dir $@)  # Ensure build subdirectories exist
	@echo "Compiling $< to $@..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning up old build..."
	@rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Cleaning completed."

debug: CXXFLAGS += $(DBEUG_FLAGS)
debug: all