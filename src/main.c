#include "../include/vm.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc < 1) {
        printf("Usage: uvm [input_file]\n");
        exit(EXIT_FAILURE);
        
    }
    FILE* file = fopen(argv[1], "rb");
    if (file == NULL) {
        printf("Failure in opening the file\n");
        exit(EXIT_FAILURE);
    }
    fseek(file, 0, SEEK_END);
    size_t len = ftell(file);
    rewind(file);
    struct vm vm;
    init_vm(&vm);
    fread(vm.memory, 1, len, file);
    //vm.memory[0].as_u32 =  OP_ADDI << 24 | R0 << 19 | R1 << 14 | 12;
    run(&vm);
    printf("r1: %d\n", vm.regs[R1].as_int);
    return 0;
}
