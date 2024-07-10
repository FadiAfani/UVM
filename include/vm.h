#ifndef VM_H
#define VM_H

#include <stdint.h>

#define MEM_SIZE (1 << 18)
#define STACK_SIZE (1 << 10)
#define READ_INST(vm) (vm->memory[ vm->regs[RIP].as_u32++ ])
#define READ_16(vm) (READ_INST(vm) | READ_INST(vm) << 8)
#define READ_24(vm) (READ_INST(vm) | READ_INST(vm) << 8 | READ_INST(vm) << 16)
#define STACK_PUSH(vm, value) ({ \
    vm->regs[RIP].as_u32--; \
    vm->memory[ vm->regs[RSP].as_u32 ] = value; \
})

#define OPCODE_MASK 0xFF000000
#define RD_MASK 0xF80000
#define RA_MASK 0x7C000
#define RB_MASK 0x3E00
#define IMM_MASK 0x3FFF

#define GET_RD(inst) ( (RD_MASK & inst) >> 19 )
#define GET_RA(inst) ( (RA_MASK & inst) >> 14 )
#define GET_RB(inst) ( (RB_MASK & inst) >> 9 )
#define GET_IMM14(inst) ( ((1 << 14) - 1) & inst )
#define GET_IMM19(inst) ( ((1 << 19) - 1) & inst )
#define GET_IMM24(inst) ( ((1 << 24) - 1) & inst )

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


typedef enum interupt {
    INTERUPT_STACK_OVERFLOW,
    INTERUPT_STACK_UNDERFLOW,
    INTERUPT_ILLEGAL_ACCESS
}Interupt;

enum opcode {
    OP_ADD,
    OP_ADDI,
    OP_FADD,
    OP_SUB,
    OP_SUBI,
    OP_FSUB,
    OP_MULT,
    OP_FMULT,
    OP_DIV,
    OP_FDIV,
    OP_MOV,
    OP_CALL,
    OP_POP,
    OP_RET,
    OP_CMP,
    OP_FCMP,
    OP_JMP,
    OP_JE,
    OP_JL,
    OP_JB,
    OP_HALT,
    OP_JNE,
    OP_JBE,
    OP_JLE,
    OP_MOVI,
    OP_LDR,
    OP_STR

};

/* Binary Instruction Encoding (Register)
 *
 * OPCODE 31 - 24
 * RD 23 - 19
 * RA 18 - 14
 * RB 13 - 9
 * 00000000 8 - 0
 *
 * */

typedef union word {
    uint8_t as_u8;
    uint16_t as_u16;
    uint32_t as_u32;
    uint64_t as_u64;
    int as_int;
    float as_float;
    double as_double;
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
    uint32_t memory[MEM_SIZE];
};

void init_vm(struct vm* vm);
void run(struct vm* vm);

#endif
