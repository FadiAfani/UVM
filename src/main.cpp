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
    JITCompiler<X64::Assembler> jit;
    jit.get_assembler().init_mmem();
    jit.get_vm().load_binary_file(argv[1]);
    jit.run();
    jit.dump_output_into_file("binary_dump");
    return 0;
}
