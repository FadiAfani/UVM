#include "../include/jit.h"
#include <stdlib.h>
#include <assert.h>
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

std::unordered_map<uint32_t, ExitData>& Trace::get_exits() { return this->exits; }


exec_func Trace::get_func() { 
    return this->func; 
}

std::unordered_set<Reg>& Trace::get_saved_regs() {
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

float Trace::get_freq() {
    if (this->trials == 0) return 1;
    return (float)(this->execs)/this->trials;
}

long Trace::exec() {
    long v = -2;
    if (this->func != nullptr) {
        v = this->func();
        if (v == 0)
            this->execs++;
        this->trials++;
    }
    return v;
}


