#ifndef JIT_H
#define JIT_H

#include <stdint.h>
#include "../lib/vector.h"
#include "../include/common.h"

#define IND_STRINGIFY(x) #x
#define STRINGIFY(x) IND_STRINGIFY(x)
#define LOAD_VM_REG_INTO_X64(vm, vm_reg, cpu_reg)  asm("mov %0, %%" STRINGIFY(cpu_reg) : : "m" (vm.regs[vm_reg].as_u64))
#define ALLOC_JMP_DATA(ptr) (ALLOCATE(ptr, sizeof(struct jmp_data), 1))


struct jmp_data {
    uint32_t disp; // label location relative to mmem
    int8_t ismapped;
    size_t instpos;
};

void dump_output_into_file(const char* fn, uint8_t* buff, size_t len);
void gen_x64(struct vm* vm, Vector* bytecode);
void gen_arm(struct vm* vm, Vector* bytecode);
void load_vm_reg_into_x64(struct vm* vm, uint cpu_reg, Reg vm_reg);


#endif
