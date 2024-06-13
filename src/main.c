#include "../include/vm.h"
#include <stdio.h>

int main(int argc, char** argv) {
    struct vm vm;
    init_vm(&vm);
    vm.memory[0].as_u32 =  OP_ADDI << 24 | R0 << 19 | R1 << 14 | 12;
    run(&vm);
    return 0;
}
