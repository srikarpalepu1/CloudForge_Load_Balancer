CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Werror -Iinclude

TARGET := cloudforge_lb.exe
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(patsubst src/%.cpp,build/%.o,$(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

build:
	mkdir -p build

run: $(TARGET)
	./$(TARGET)

docs:
	doxygen Doxyfile

clean:
	rm -rf build $(TARGET)

.PHONY: all run docs clean