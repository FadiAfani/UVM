#include "../include/jit.h"
#include <fstream>
#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <utility>
#include <vector>


void transfer_reg_x64(X64::Assembler& x64_asm, VM* vm, unsigned int cpu_reg, Reg vm_reg, bool to_cpu) {
    /* mov rax, imm64
     * mov cpu_reg, [rax]
     * */
    X64::Register rax(X64::RAX, 8);
    X64::Register reg(cpu_reg, 8);
    X64::MemOp<> memop(rax);
    if (to_cpu) {
        x64_asm.mov(reg, vm->get_reg(vm_reg));
    }
    else {
        x64_asm.mov(rax, &vm->get_reg(vm_reg));
        x64_asm.mov(reg, memop);
    }

}

int Trace::get_heat() { 
    return this->heat; 
}

exec_func Trace::get_func() { 
    return this->func; 
}

std::vector<Reg>& Trace::get_saved_regs() {
    return this->saved_regs;
}

const std::vector<uint32_t>& Trace::get_bytecode() { 
    return this->bytecode; 
}

void Trace::set_func(exec_func func) {
    this->func = func; 
}

void Trace::push_inst(uint32_t inst) {
    this->bytecode.push_back(inst);
}

void Trace::push_path(std::unique_ptr<Trace> trace) {
    this->paths.push_back(std::move(trace));
}

void Trace::inc_heat() {
    this->heat++;
}






