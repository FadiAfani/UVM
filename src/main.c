#include "../include/vm.h"
#include "../include/jit.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <assert.h>

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

    Vector* vec = gen_x64(&vm, 0, len/4);
    printf("size: %ld\n", vec->size);
    void* mcode = mmap(NULL, vec->size * vec->esize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
    memcpy(mcode, vec->arr, vec->size * vec->esize);
    int (*func)() = mcode;
    printf("out: %d\n", func());


    //run(&vm);
    //printf("r1: %d\n", vm.regs[R1].as_int);
    return 0;
}
