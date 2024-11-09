# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++23 -I./src -I./include -Wextra -Wall -g

# Source files and includes
SOURCES = $(shell find ./src -name '*.cpp')
INCLUDES = $(shell find ./src -type d -exec printf '-I%s ' {} \;) $(shell find ./include -type d -exec printf '-I%s ' {} \;)
TARGET = main

# Declare phony targets
.PHONY: all clean test

# Build the main target
all: $(TARGET)

$(TARGET):
	@echo "Building $(TARGET)..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(SOURCES)

# Clean up the build
clean:
	@echo "Cleaning up..."
	@rm -f $(TARGET) test

# Test build
test:
	@echo "Building test executable..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o test ./src/vmtest.cpp ./src/Lexer/lexer.cpp ./src/Lexer/token.cpp $(shell find ./src/VM -name '*.cpp')
