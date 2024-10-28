CXX = g++
CXXFLAGS = -std=c++23 -I./src -I./include
SOURCES = $(shell find ./src -name '*.cpp')
TARGET = main.exe

all: $(TARGET)

$(TARGET):
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)
