#include <criterion/criterion.h>
#include <sys/mman.h>
#include "../include/jit.h"

struct vm vm;

#define EXEC(mc, ret_t)

void setup(void) {
    init_vm(&vm);
}

TestSuite(jit_tests, .init=setup);

Test(jit_tests, test_ldr) {
    vm.memory[10] = 42;
    vm.regs[R1].as_u64 = 40; // sizeof(uint32_t) * 10
    vm.memory[0] = OP_LDR << 24 | R0 << 19 | R1 << 14;
    vm.memory[1] = OP_RET << 24;
    Vector mc;
    INIT_VECTOR(mc, 1);
    load_vm_reg_into_x64(&vm, 0x3, R1, &mc);
    gen_x64(&vm, &mc, 0, 2);
    void* mcode = mmap(NULL, mc.size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
    memcpy(mcode, mc.arr, mc.size);
    uint32_t (*f)() = mcode;
    uint32_t ret = f();
    cr_expect_eq(ret, 42);
    

}
