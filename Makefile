CXX := g++
CXXFLAGS := -Wall -O3 -Wextra -std=c++20 -Iinclude
# CXXFLAGS := -Wall -Wextra -std=c++20 -Iinclude
LDFLAGS :=

SRC := $(wildcard src/*.cpp)
OBJ := $(patsubst src/%.cpp, build/%.o, $(SRC))
TARGET := bin/sweeper

TEST_SRC := $(wildcard unit/*.cpp)
TEST_OBJ := $(patsubst unit/%.cpp, build/unit/%.o, $(TEST_SRC))
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

$(TEST_TARGET): $(TEST_OBJ) $(filter-out build/main.o, $(OBJ)) | bin build/unit
	$(CXX) $(TEST_OBJ) $(filter-out build/main.o, $(OBJ)) -o $@ $(LDFLAGS)

build/unit/%.o: unit/%.cpp | build/unit
	$(CXX) $(CXXFLAGS) -Ithird -c $< -o $@

build/unit:
	mkdir -p build/unit

.PHONY: all clean test
