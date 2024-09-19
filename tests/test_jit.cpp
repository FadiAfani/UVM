#include <criterion/criterion.h>
#include "../include/jit.h"

JITCompiler<X64::Assembler> jit;
VM& vm = jit.get_vm();
X64::Assembler& assembler = jit.get_assembler();

TestSuite(jit_tests);

/* moves value from vm register to cpu register 
 * adds 5 at the cpu level 
 * then transfers the register state back to the vm
 * */
Test(test_state_transfer_x64, jit_tests) {
    assembler.init_mmem();
    Word& r = vm.get_reg_as_ref(R0);
    r.as_int = 3;
    transfer_reg_x64(assembler, &vm, RBX, R0, true);
    std::vector<uint32_t> bc = {
        OP_ADDI << 24 | R1 << 19 | R1 << 14 | 5,
    };
    jit.gen_x64(bc);
    jit.dump_output_into_file("binary_dump");
    //func();
    cr_expect_eq(r.as_int, 8);


}

