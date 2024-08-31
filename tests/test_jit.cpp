#include <criterion/criterion.h>
#include "../include/jit.h"

struct vm vm;

#define EXEC(ret, ret_t, len) ({ \
    gen_x64(&vm, 0, len); \
    ret_t (*f)() = (void*) vm.mmem; \
    ret = f(); \
})

void setup(void) {
    init_vm(&vm);
}

TestSuite(jit_tests, .init=setup);

Test(jit_tests, test_ldr) {
    vm.memory[10] = 42;
    vm.regs[R1].as_u64 = 40; // sizeof(uint32_t) * 10
    vm.memory[0] = OP_LDR << 24 | R0 << 19 | R1 << 14;
    vm.memory[1] = OP_RET << 24;
    load_vm_reg_into_x64(&vm, 0x3, R1);
    uint32_t ret;
    EXEC(ret, uint32_t, 2);
    cr_expect_eq(ret, 42);

}

Test(jit_tests, test_str) {
    vm.regs[R0].as_u64 = 42;
    vm.regs[R1].as_u64 = 40; // sizeof(uint32_t) * 10
    vm.memory[0] = OP_STR << 24 | R0 << 19 | R1 << 14;
    vm.memory[1] = OP_RET << 24;
    load_vm_reg_into_x64(&vm, 0x0, R0);
    load_vm_reg_into_x64(&vm, 0x3, R1);
    uint32_t ret;
    EXEC(ret, uint32_t, 2);
    cr_expect_eq(vm.memory[10], 42);
}

Test(jit_tests, test_cmp) {
    vm.regs[R0].as_u64 = 10;
    vm.regs[R1].as_u64 = 20;
    vm.memory[0] = OP_CMP << 24 | R0 << 19 | R1 << 14;
    vm.memory[1] = OP_RET << 24;
    load_vm_reg_into_x64(&vm, 0x0, R0);
    load_vm_reg_into_x64(&vm, 0x3, R1);
    uint8_t ret;
    EXEC(ret, uint8_t, 2);
    asm("setb %%al\n\t" 
        "mov %0, %%al"
            : "=r"(ret));
    cr_expect_eq(ret, 1);

}
