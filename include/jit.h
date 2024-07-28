#ifndef JIT_H
#define JIT_H

#include "../lib/vector.h"
#include "../include/vm.h"


void gen_x64(struct vm* vm, Vector* tcode, size_t addr, size_t len);
void gen_arm(struct vm* vm, Vector* tcode, size_t addr, size_t len);
void load_vm_reg_into_x64(struct vm* vm, uint cpu_reg, Reg vm_reg, Vector* vec);

#endif
