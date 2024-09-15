BUILD = build
SRC = src
BIN = bin
TESTS = tests
TEST_BINS = $(TESTS)/bin
CFLAGS = -Wall -Wextra -g
CC = g++

SRCS = $(wildcard $(SRC)/*.cpp)
OBJS = $(SRCS:$(SRC)/%.cpp=$(BUILD)/%.o)
TEST_SRCS = $(wildcard $(TESTS)/*.c)

$(BUILD)/%.o: $(SRC)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

all: $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN)/uvm $^

test_vm: $(TESTS)/test_vm.cpp $(BUILD)/vm.o $(BUILD)/jit.o
	$(CC) -o $(TEST_BINS)/$@ $^ -lcriterion
	./$(TEST_BINS)/$@


test_jit: $(TESTS)/test_jit.cpp $(BUILD)/vm.o $(BUILD)/jit.o 
	$(CC) -o $(TEST_BINS)/$@ $^ -lcriterion
	./$(TEST_BINS)/$@



clear_bin:
	rm -rf $(BIN)/*

clear_build:
	rm -rf $(BUILD)/*
