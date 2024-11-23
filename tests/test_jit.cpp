#include <criterion/criterion.h>
#include "../include/jit.h"

JITCompiler<X64::Assembler> jit;
VM& vm = jit.get_vm();

UVMAssembler uvm_asm(vm);

TestSuite(jit_tests);

std::string input;

Test(test_one_branch, jit_tests) {
    jit.get_assembler().init_mmem();

    uvm_asm.movi(R0, 0);
    uvm_asm.movi(R1, 10);
    uvm_asm.addi(R0, R0, 1);
    uvm_asm.cmp(R0, R1);
    uvm_asm.jl(2);
    uvm_asm.ret();

    jit.run();
    Trace* t = jit.get_tracer().get_trace(3);
    cr_expect_neq(t, nullptr);
    cr_expect(t->get_trials() > 0);
    cr_expect_eq(vm.get_reg(R2).as_u64, 10);

}

