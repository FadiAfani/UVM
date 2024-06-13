#include <criterion/criterion.h>
#include "../include/vm.h"

struct vm vm;

void setup(void) {
    init_vm(&vm);
}


TestSuite(vm_tests, .init=setup);

Test(vm_tests, test_addi) {
    vm.memory[0].as_u32 = OP_ADDI << 24 | R0 << 19 | R1 << 14 | 100;
    run(&vm);
    cr_expect(vm.regs[R0].as_int == 100, "R0 should have hold a value of 100");
}


Test(vm_tests, test_add) {
    vm.regs[R1].as_int = 20;
    vm.regs[R2].as_int = 42;
    vm.memory[0].as_u32 = OP_ADD << 24 | R0 << 19 | R1 << 14 | R2 << 9;
    run(&vm);
    cr_expect(vm.regs[R0].as_int == 62, "R0 should have hold a value of 62");
}


Test(vm_tests, test_sub) {
    vm.regs[R1].as_int = 20;
    vm.regs[R2].as_int = 42;
    vm.memory[0].as_u32 = OP_SUB << 24 | R0 << 19 | R1 << 14 | R2 << 9;
    run(&vm);
    cr_expect(vm.regs[R0].as_int == -22, "R0 should have hold a value of -22");
}


Test(vm_tests, test_mult) {
    vm.regs[R1].as_int = 10;
    vm.regs[R2].as_int = 5;
    vm.memory[0].as_u32 = OP_MULT << 24 | R0 << 19 | R1 << 14 | R2 << 9;
    run(&vm);
    cr_expect(vm.regs[R0].as_int == 50, "R0 should have hold a value of 50");
}

