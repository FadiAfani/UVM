#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include "../lib/vector.h"
#include "../lib/hash_table.h"


#define MEM_SIZE (1 << 18)


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


typedef void (*mcfunc)();

typedef struct trace {
    uint32_t saddr;
    int heat;
    uint8_t* mmem_ptr;
    mcfunc exec_func;
    Vector bytecode;
    Vector sub_paths;
}Trace;


typedef struct loop_header {
    int heat;
    Trace* trace;
}LoopHeader;

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
    OP_PUSH,
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
    int32_t as_int;
    float as_float;
    double as_double;
}Word;

/* R0-R7 general purpose 
 * R8-R15 floating-point 
 * RIP - instruction pointer
 * RFLG - condition result
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
    RIP,
    RFLG,
}Reg;

struct vm {
    Word regs[RFLG + 1];
    uint32_t memory[MEM_SIZE];
    uint8_t* mmem; 
    size_t mmem_size;
    size_t mmem_cap;
    /* maps instruction pointers to counters */
    HashTable loop_headers;
    Trace* tp;
    bool is_tracing;
    bool native_exec;
};



#endif
