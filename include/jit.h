#ifndef JIT_H
#define JIT_H

#include <stdint.h>
#include "../lib/vector.h"
#include "../include/common.h"

#define IND_STRINGIFY(x) #x
#define STRINGIFY(x) IND_STRINGIFY(x)
#define LOAD_VM_REG_INTO_X64(vm, vm_reg, cpu_reg)  asm("mov %0, %%" STRINGIFY(cpu_reg) : : "m" (vm.regs[vm_reg].as_u64))
#define ALLOC_JMP_DATA(ptr) (ALLOCATE(ptr, sizeof(struct jmp_data), 1))

#define TARGET_CPU 0
#define TARGET_VM 1

typedef enum arch_type {
    X86_64,
}ArchType;


struct jmp_data {
    uint32_t disp; // label location relative to mmem
    int8_t ismapped;
    size_t instpos;
};

void dump_output_into_file(const char* fn, uint8_t* buff, size_t len);
void gen_x64(struct vm* vm, Vector* bytecode);
void gen_arm(struct vm* vm, Vector* bytecode);
void transfer_reg_x64(struct vm* vm, unsigned int cpu_reg, Reg vm_reg, bool to_cpu);
void transfer_reg_state(struct vm* vm, ArchType arch, bool to_cpu);

#endif
