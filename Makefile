BUILD = build
SRC = src
BIN = bin
CFLAGS = -Wall -Wextra -o -g

OBJS = $(wildcard $(BUILD)/*.o)
SRCS = $(wildcard $(SRC)/*.c)

all: $(SRCS)
	gcc -o $(BIN)/uvm $^

clear_bin:
	rm -rf $(BIN)/*
