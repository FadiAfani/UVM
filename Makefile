BUILD = build
SRC = src
BIN = bin
TESTS = tests
TEST_BINS = $(TESTS)/bin
CFLAGS = -Wall -Wextra -o -g

OBJS = $(SRCS:$(SRC)/%.c=$(BUILD)/%.o)
SRCS = $(wildcard $(SRC)/*.c)
TEST_SRCS = $(wildcard $(TESTS)/*.c)

$(BUILD)/%.o: $(SRC)/%.c
	gcc -c $< -o $@

all: $(OBJS)
	gcc -o $(BIN)/uvm $^

test_vm: $(TESTS)/test_vm.c $(BUILD)/vm.o
	gcc -o $(TEST_BINS)/$@ $^ -lcriterion
	./$(TEST_BINS)/$@



clear_bin:
	rm -rf $(BIN)/*
