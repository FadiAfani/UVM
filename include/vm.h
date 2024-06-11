#ifndef VM_H
#define VM_H

#include <stdint.h>

#define MEM_SIZE (1 << 16)
#define STACK_SIZE (1 << 10)
#define READ_INST(vm) (vm->memory[ vm->regs[RIP].as_int++ ])

enum opcode {
    OP_ADD,
    OP_FADD,
    OP_SUB,
    OP_FSUB,
    OP_MULT,
    OP_FMULT,
    OP_DIV,
    OP_FDIV,
    OP_XOR,
    OP_MOV,
    OP_CALL,
    OP_POP,
    OP_RET,
    OP_JMP,
    OP_JIF
};

typedef union word {
    uint8_t as_u8;
    uint16_t as_u16;
    uint32_t as_u32;
    uint64_t as_u64;
    int as_int;
    float as_float;
    double as_double;
    void* as_ref;
}Word;

/* R0-R7 general purpose 
 * R8-R15 floating-point 
 * RSP - stack pointer
 * RBP - frame pointer
 * RIP - instruction pointer
 * RFLG - condition result
 * RAX - return value
 * */

typedef enum reg {
    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    RSP,
    RBP,
    RIP,
    RFLG,
    RAX,
}Reg;

struct vm {
    Word regs[RAX + 1];
    Word memory[MEM_SIZE];
};

void init_vm(struct vm* vm);
void run(struct vm* vm);

#endif
