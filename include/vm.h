#ifndef VM_H
#define VM_H

#include <stdint.h>
#include <stdbool.h>
#include "../include/common.h"
#include "../include/jit.h"


#define STACK_SIZE (1 << 10)
#define READ_INST(vm) (vm->memory[ vm->regs[RIP].as_u32++ ])
#define READ_16(vm) (READ_INST(vm) | READ_INST(vm) << 8)
#define READ_24(vm) (READ_INST(vm) | READ_INST(vm) << 8 | READ_INST(vm) << 16)
#define STACK_PUSH(vm, value) ( vm->memory[ --vm->regs[R7].as_u32 ] = value )


#define BIN_OP_REG(vm, inst, as_type, op) ({ \
    unsigned int rd = (RD_MASK & inst) >> 19; \
    unsigned int ra = (RA_MASK & inst) >> 14; \
    unsigned int rb = (RB_MASK & inst) >> 9; \
    vm->regs[rd].as_type = vm->regs[ra].as_type op vm->regs[rb].as_type; \
})

#define BIN_OP_IMM(vm, inst, as_type, op) ({ \
    unsigned int rd = (RD_MASK & inst) >> 19; \
    unsigned int ra = (RA_MASK & inst) >> 14; \
    uint16_t imm = (IMM_MASK & inst); \
    vm->regs[rd].as_type = vm->regs[ra].as_type op imm; \
})

#define ALLOC_TRACE(ptr) (ALLOCATE(ptr, sizeof(Trace), 1))
#define INIT_TRACE(trace) ({ \
    trace.saddr = 0; \
    trace.heat = 0; \
    trace.func = NULL; \
    INIT_VECTOR(trace.bytecode, 4); \
})

#define INIT_LOOP_HEADER(lh) ({ \
    lh.heat = 0; \
    lh.trace = NULL; \
})


typedef enum interrupt {
    INTERRUPT_STACK_OVERFLOW,
    INTERRUPT_STACK_UNDERFLOW,
    INTERRUPT_ILLEGAL_ACCESS
}Interrupt;


void write_trace(struct vm* vm, Trace* trace);
void init_vm(struct vm* vm);
void run(struct vm* vm);

#endif
