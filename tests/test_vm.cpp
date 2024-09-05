#include <criterion/criterion.h>
#include "../include/vm.h"

VM vm(false);

void setup(void) {

}


TestSuite(vm_tests, .init=setup);

Test(vm_tests, test_addi) {
    vm.set_memory_addr(0, OP_ADDI << 24 | R0 << 19 | R1 << 14 | 100);
    vm.set_memory_addr(1, OP_HALT << 24);
    vm.run();
    cr_expect(vm.get_reg(R0).as_int == 100, "R0 should have hold a value of 100");
}


Test(vm_tests, test_add) {
    vm.get_reg(R1).as_int = 20;
    vm.get_reg(R2).as_int = 42;
    vm.set_memory_addr(0, OP_ADD << 24 | R0 << 19 | R1 << 14 | R2 << 9);
    vm.set_memory_addr(1, OP_HALT << 24);
    vm.run();
    cr_expect(vm.get_reg(R0).as_int == 62, "R0 should have hold a value of 62");
}


Test(vm_tests, test_sub) {
    vm.get_reg(R1).as_int = 20;
    vm.get_reg(R2).as_int = 42;
    vm.set_memory_addr(0, OP_SUB << 24 | R0 << 19 | R1 << 14 | R2 << 9);
    vm.set_memory_addr(1, OP_HALT << 24);
    vm.run();
    cr_expect(vm.get_reg(R0).as_int == -22, "R0 should have hold a value of -22");
}


Test(vm_tests, test_mov) {
    vm.get_reg(R1).as_int = 20;
    vm.set_memory_addr(0, OP_MOV << 24 | R0 << 19 | R1 << 14);
    vm.set_memory_addr(1, OP_HALT << 24);
    vm.run();
    cr_expect(vm.get_reg(R0).as_int == 20, "R0 should have hold a value of 20");
}


Test(vm_tests, test_cmp_lt) {
    vm.get_reg(R0).as_int = 10;
    vm.get_reg(R1).as_int = 20;
    vm.set_memory_addr(0, OP_CMP << 24 | R0 << 19 | R1 << 14);
    vm.set_memory_addr(1, OP_HALT << 24);
    vm.run();
    cr_expect(vm.get_reg(RFLG).as_int == -1, "RFLG should have hold a value of 1");

}

Test(vm_tests, test_cmp_eq) {

    vm.get_reg(R0).as_int = 20;
    vm.get_reg(R1).as_int = 20;
    vm.set_memory_addr(0, OP_CMP << 24 | R0 << 19 | R1 << 14);
    vm.set_memory_addr(1, OP_HALT << 24);
    vm.run();
    cr_expect(vm.get_reg(RFLG).as_int == 0, "RFLG should have hold a value of 0");
}


Test(vm_tests, test_cmp_bt) {

    vm.get_reg(R0).as_int = 30;
    vm.get_reg(R1).as_int = 20;
    vm.set_memory_addr(0, OP_CMP << 24 | R0 << 19 | R1 << 14);
    vm.set_memory_addr(1, OP_HALT << 24);
    vm.run();
    cr_expect(vm.get_reg(RFLG).as_int == 1, "RFLG should have hold a value of 1");

}

Test(vm_tests, test_movi) {
    vm.set_memory_addr(0, OP_MOVI << 24 | R1 << 19 | 15);
    vm.set_memory_addr(1, OP_HALT << 24);
    vm.run();
    cr_expect(vm.get_reg(R1).as_u32 == 15, "R1 should hold a value of 15");
}

Test(vm_tests, test_str) {
    vm.get_reg(R2).as_u32 = 5;
    vm.get_reg(R1).as_u32 = 12;
    vm.set_memory_addr(0, OP_STR << 24 | R1 << 19 | R2 << 14);
    vm.set_memory_addr(1, OP_HALT << 24);
    vm.run();
    cr_expect(vm.read_memory(5) == 12, "memory address should hold a value of 50");
}

Test(vm_tests, test_ldr) {
    vm.get_reg(R2).as_u32 = 2;
    vm.set_memory_addr(0, OP_LDR << 24 | R1 << 19 | R2 << 14);
    vm.set_memory_addr(1, OP_HALT << 24);
    vm.set_memory_addr(2, 50);
    vm.run();
    cr_expect(vm.get_reg(R1).as_u32 == 50, "R1 should hold a value of 50");
}



