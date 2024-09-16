#ifndef VM_H
#define VM_H

#include <fstream>
#include <stdint.h>
#include <stdbool.h>
#include "../include/common.h"


#define MEM_SIZE (1 << 18)

#define OPCODE_MASK 0xFF000000
#define RD_MASK 0xF80000
#define RA_MASK 0x7C000
#define RB_MASK 0x3E00
#define IMM_MASK 0x3FFF


#define GET_OPCODE(inst) ( (inst & OPCODE_MASK) >> 24)
#define GET_RD(inst) static_cast<Reg>( (RD_MASK & inst) >> 19 )
#define GET_RA(inst) static_cast<Reg>( (RA_MASK & inst) >> 14 )
#define GET_RB(inst) static_cast<Reg>( (RB_MASK & inst) >> 9 )

#define GET_IMM(inst, bits) static_cast<Reg>( ((1 << bits) - 1) & inst )
#define GET_IMM14(inst) ( GET_IMM(inst, 14) )
#define GET_IMM19(inst) ( GET_IMM(inst, 19) )
#define GET_IMM24(inst) ( GET_IMM(inst, 24) )


typedef void (*exec_func)(); 


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

typedef enum interrupt {
    INTERRUPT_STACK_OVERFLOW,
    INTERRUPT_STACK_UNDERFLOW,
    INTERRUPT_ILLEGAL_ACCESS
}Interrupt;

template<typename T>
class JITCompiler;

class Interpreter {

};

class VM {
    Word regs[RFLG + 1];
    uint32_t memory[MEM_SIZE];

    public:
        VM();
        Word& get_reg_as_ref(Reg r);
        template<typename T>
        JITCompiler<T>* get_jit();
        uint32_t fetch();
        uint8_t decode(uint32_t inst);
        uint32_t read_memory(uint32_t addr);
        void save_mod_regs(uint32_t inst);
        void set_memory_addr(uint32_t addr, uint32_t value);
        int interpret(uint32_t inst);
        void load_binary_file(const char* fn);
        void run();
};


#endif
