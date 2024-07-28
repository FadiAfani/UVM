BUILD = build
SRC = src
BIN = bin
TESTS = tests
TEST_BINS = $(TESTS)/bin
CFLAGS = -Wall -Wextra -g

OBJS = $(SRCS:$(SRC)/%.c=$(BUILD)/%.o)
SRCS = $(wildcard $(SRC)/*.c)
TEST_SRCS = $(wildcard $(TESTS)/*.c)

$(BUILD)/%.o: $(SRC)/%.c
	gcc $(CFLAGS) -c $< -o $@

all: $(OBJS)
	gcc $(CFLAGS) -o $(BIN)/uvm $^

test_vm: $(TESTS)/test_vm.c $(BUILD)/vm.o
	gcc -o $(TEST_BINS)/$@ $^ -lcriterion
	./$(TEST_BINS)/$@


test_jit: $(TESTS)/test_jit.c $(BUILD)/vm.o $(BUILD)/jit.o $(BUILD)/vector.o
	gcc -o $(TEST_BINS)/$@ $^ -lcriterion
	./$(TEST_BINS)/$@



clear_bin:
	rm -rf $(BIN)/*

clear_build:
	rm -rf $(BUILD)/*
