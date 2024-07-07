#include <criterion/criterion.h>
#include "../include/vm.h"
#include <stdio.h>

struct vm vm;

void setup(void) {
    init_vm(&vm);
}


TestSuite(vm_tests, .init=setup);

Test(vm_tests, test_addi) {
    vm.memory[0] = OP_ADDI << 24 | R0 << 19 | R1 << 14 | 100;
    vm.memory[1] = OP_HALT << 24;
    run(&vm);
    cr_expect(vm.regs[R0].as_int == 100, "R0 should have hold a value of 100");
}


Test(vm_tests, test_add) {
    vm.regs[R1].as_int = 20;
    vm.regs[R2].as_int = 42;
    vm.memory[0] = OP_ADD << 24 | R0 << 19 | R1 << 14 | R2 << 9;
    vm.memory[1] = OP_HALT << 24;
    run(&vm);
    cr_expect(vm.regs[R0].as_int == 62, "R0 should have hold a value of 62");
}


Test(vm_tests, test_sub) {
    vm.regs[R1].as_int = 20;
    vm.regs[R2].as_int = 42;
    vm.memory[0] = OP_SUB << 24 | R0 << 19 | R1 << 14 | R2 << 9;
    vm.memory[1] = OP_HALT << 24;
    run(&vm);
    cr_expect(vm.regs[R0].as_int == -22, "R0 should have hold a value of -22");
}


Test(vm_tests, test_mov) {
    vm.regs[R1].as_int = 20;
    vm.memory[0] = OP_MOV << 24 | R0 << 19 | R1 << 14;
    vm.memory[1] = OP_HALT << 24;
    run(&vm);
    cr_expect(vm.regs[R0].as_int == 20, "R0 should have hold a value of 20");
}


Test(vm_tests, test_cmp_lt) {
    vm.regs[R0].as_int = 10;
    vm.regs[R1].as_int = 20;
    vm.memory[0] = OP_CMP << 24 | R0 << 19 | R1 << 14;
    vm.memory[1] = OP_HALT << 24;
    run(&vm);
    cr_expect(vm.regs[RFLG].as_int == -1, "RFLG should have hold a value of 1");

}

Test(vm_tests, test_cmp_eq) {

    vm.regs[R0].as_int = 20;
    vm.regs[R1].as_int = 20;
    vm.memory[0] = OP_CMP << 24 | R0 << 19 | R1 << 14;
    vm.memory[1] = OP_HALT << 24;
    run(&vm);
    cr_expect(vm.regs[RFLG].as_int == 0, "RFLG should have hold a value of 0");
}


Test(vm_tests, test_cmp_bt) {

    vm.regs[R0].as_int = 30;
    vm.regs[R1].as_int = 20;
    vm.memory[0] = OP_CMP << 24 | R0 << 19 | R1 << 14;
    vm.memory[1] = OP_HALT << 24;
    run(&vm);
    cr_expect(vm.regs[RFLG].as_int == 1, "RFLG should have hold a value of 1");

}


Test(vm_tests, test_jmp) {

    vm.memory[0] = OP_JMP << 24 | 41;
    vm.memory[1] = OP_HALT << 24;
    run(&vm);
    cr_expect(vm.regs[RIP].as_u32 == 41, "RIP should have hold a value of 41");
}


Test(vm_tests, test_movi) {
    vm.memory[0] = OP_MOVI << 24 | R1 << 19 | 15;
    vm.memory[1] = OP_HALT << 24;
    run(&vm);
    cr_expect(vm.regs[R1].as_u32 == 15, "R1 should hold a value of 15");
}



