#ifndef JIT_H
#define JIT_H

#include "../lib/vector.h"
#include "../include/vm.h"

#define IND_STRINGIFY(x) #x
#define STRINGIFY(x) IND_STRINGIFY(x)
#define LOAD_VM_REG_INTO_X64(vm, vm_reg, cpu_reg)  asm("mov %0, %%" STRINGIFY(cpu_reg) : : "m" (vm.regs[vm_reg].as_u64))


void gen_x64(struct vm* vm, size_t addr, size_t len);
void gen_arm(struct vm* vm, size_t addr, size_t len);
void load_vm_reg_into_x64(struct vm* vm, uint cpu_reg, Reg vm_reg);


#endif
