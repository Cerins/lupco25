CXX := g++
CXXFLAGS := -Wall -O3 -Wextra -std=c++20 -Iinclude
LDFLAGS :=

SRC := $(wildcard src/*.cpp)
OBJ := $(patsubst src/%.cpp, build/%.o, $(SRC))
TARGET := bin/sweeper

TEST_SRC := $(wildcard tests/*.cpp)
TEST_OBJ := $(patsubst tests/%.cpp, build/tests/%.o, $(TEST_SRC))
TEST_TARGET := bin/test_runner

all: $(TARGET)

$(TARGET): $(OBJ) | bin
	$(CXX) $(OBJ) -o $@ $(LDFLAGS)

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build/* bin/*

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJ) $(filter-out build/main.o, $(OBJ)) | bin build/tests
	$(CXX) $(TEST_OBJ) $(filter-out build/main.o, $(OBJ)) -o $@ $(LDFLAGS)

build/tests/%.o: tests/%.cpp | build/tests
	$(CXX) $(CXXFLAGS) -Ithird -c $< -o $@

build/tests:
	mkdir -p build/tests

.PHONY: all clean test
