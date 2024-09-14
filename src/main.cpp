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
    JITCompiler jit;
    jit.init_mmem();
    jit.get_vm().load_binary_file(argv[1]);
    jit.run();
    return 0;
}
