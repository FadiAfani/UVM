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

#define OPCODE_MASK 0xFF000000
#define RD_MASK 0xF80000
#define RA_MASK 0x7C000
#define RB_MASK 0x3E00
#define IMM_MASK 0x3FFF

#define GET_REG_AS(reg, type) (vm->regs[RIP].type)

#define GET_OPCODE(inst) ( (inst & OPCODE_MASK) >> 24)
#define GET_RD(inst) ( (RD_MASK & inst) >> 19 )
#define GET_RA(inst) ( (RA_MASK & inst) >> 14 )
#define GET_RB(inst) ( (RB_MASK & inst) >> 9 )

#define GET_IMM(inst, bits) ( ((1 << bits) - 1) & inst )
#define GET_IMM14(inst) ( GET_IMM(inst, 14) )
#define GET_IMM19(inst) ( GET_IMM(inst, 19) )
#define GET_IMM24(inst) ( GET_IMM(inst, 24) )

#define BIN_OP_REG(vm, inst, as_type, op) ({ \
    Reg rd = (RD_MASK & inst) >> 19; \
    Reg ra = (RA_MASK & inst) >> 14; \
    Reg rb = (RB_MASK & inst) >> 9; \
    vm->regs[rd].as_type = vm->regs[ra].as_type op vm->regs[rb].as_type; \
})

#define BIN_OP_IMM(vm, inst, as_type, op) ({ \
    Reg rd = (RD_MASK & inst) >> 19; \
    Reg ra = (RA_MASK & inst) >> 14; \
    uint16_t imm = (IMM_MASK & inst); \
    vm->regs[rd].as_type = vm->regs[ra].as_type op imm; \
})

#define ALLOC_TRACE(ptr) (ALLOCATE(ptr, sizeof(Trace), 1))
#define INIT_TRACE(trace) ({ \
    trace.saddr = 0; \
    trace.heat = 0; \
    INIT_VECTOR(trace.bytecode, 1); \
    INIT_VECTOR(trace.sub_paths, sizeof(Trace)); \
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
