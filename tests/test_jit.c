#include <criterion/criterion.h>
#include <sys/mman.h>
#include "../include/jit.h"

struct vm vm;
Vector mc;
void* mmem;

#define EXEC(ret, ret_t, len) ({ \
    gen_x64(&vm, &mc, 0, len); \
    mmem = mmap(NULL, mc.size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0); \
    memcpy(mmem, mc.arr, mc.size); \
    ret_t (*f)() = mmem; \
    ret = f(); \
})

void setup(void) {
    init_vm(&vm);
    INIT_VECTOR(mc, 1);
}

TestSuite(jit_tests, .init=setup);

Test(jit_tests, test_ldr) {
    vm.memory[10] = 42;
    vm.regs[R1].as_u64 = 40; // sizeof(uint32_t) * 10
    vm.memory[0] = OP_LDR << 24 | R0 << 19 | R1 << 14;
    vm.memory[1] = OP_RET << 24;
    load_vm_reg_into_x64(&vm, 0x3, R1, &mc);
    uint32_t ret;
    EXEC(ret, uint32_t, 2);
    cr_expect_eq(ret, 42);

}

Test(jit_tests, test_str) {
    vm.regs[R0].as_u64 = 42;
    vm.regs[R1].as_u64 = 40; // sizeof(uint32_t) * 10
    vm.memory[0] = OP_STR << 24 | R0 << 19 | R1 << 14;
    vm.memory[1] = OP_RET << 24;
    load_vm_reg_into_x64(&vm, 0x0, R0, &mc);
    load_vm_reg_into_x64(&vm, 0x3, R1, &mc);
    uint32_t ret;
    EXEC(ret, uint32_t, 2);
    cr_expect_eq(vm.memory[10], 42);
}
