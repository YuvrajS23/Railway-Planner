CXX ?= c++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic

TARGET := railway-planner
TEST_TARGET := data-structure-tests
PROJECT_FILES := $(wildcard *.cpp *.h data_structures/*.h)

.PHONY: all run demo test clean

all: $(TARGET)

$(TARGET): $(PROJECT_FILES)
	$(CXX) $(CXXFLAGS) main.cpp -o $@

$(TEST_TARGET): tests/data_structures_test.cpp Heap.h Heap.cpp \
		data_structures/BinarySearchTree.h data_structures/AVLTree.h
	$(CXX) $(CXXFLAGS) tests/data_structures_test.cpp -o $@

run: $(TARGET)
	./$(TARGET) planner.log

demo: $(TARGET)
	./$(TARGET) planner.log < examples/demo-session.txt

test: $(TARGET) $(TEST_TARGET)
	./$(TEST_TARGET)
	./$(TARGET) /tmp/railway-planner-test.log < examples/demo-session.txt > /dev/null
	@echo "Planner demo smoke test passed."

clean:
	rm -f $(TARGET) $(TEST_TARGET) planner.log
