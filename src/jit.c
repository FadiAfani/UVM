#include "../include/vm.h"
#include "../lib/vector.h"
#include "../include/jit.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


uint8_t x64_reg[] = {
    [R0] = 0x0,
    [R1] = 0x3,
    [R2] = 0x1,
    [R3] = 0x2,
    [R4] = 0x6,
    [R5] = 0x7,
    [R6] = 0x5,
    [R7] = 0x4,

};

/* add and mov
 * <prefix>
 * <add-opcode>
 * <mod + regs>
 * <prefix> 
 * <mov-opcode>
 * <mod + regs>
*/

#define JIT_BINOP_X64(tcode, opcode, inst) ({ \
    APPEND(tcode, 0x48, uint8_t); \
    append_arr_to_vector(&tcode, opcode, sizeof(opcode), 1); \
    uint8_t mc[] = { \
        0x03 << 6 | x64_reg[GET_RA(inst)] << 3 | x64_reg[GET_RB(inst)], \
        0x48, \
        0x89, \
        0x03 << 6 | x64_reg[GET_RB(inst)] << 3 | x64_reg[GET_RD(inst)] \
    }; \
    append_arr_to_vector(&tcode, mc, sizeof(mc), 1); \
})


/* MODR/M
 * mod -> 2 bits
 * reg -> 3 bits
 * rm  -> 3 bits
 * */
#define MOD_BYTE(mod, reg, rm) ( mod << 6 | reg << 3 | rm )

/* REX prefix */
#define REX(W,R,X,B) ( 0x40 | W << 3 | R << 2 | X << 1 | B )

void load_vm_reg_into_x64(struct vm* vm, uint cpu_reg, Reg vm_reg, Vector* vec) {
    uint8_t mc[] = {
        REX(1,0,0,0),
        0xb8 + cpu_reg,
        0,0,0,0,0,0,0,0
    };
    *(uint64_t*) (mc + 2) = vm->regs[vm_reg].as_u64;
    append_arr_to_vector(vec, mc, sizeof(mc), 1);
}


void gen_x64(struct vm* vm, Vector* tcode, size_t addr, size_t len) {
    uint32_t inst;

    //TODO: fail when addr/len > MEM_SIZE

    for (size_t i = 0; i < len; i++) {
        inst = vm->memory[addr + i];
        uint8_t opcode = GET_OPCODE(inst);
        switch(opcode) {
            case OP_MOV:
            {
                uint8_t mc[] = {
                    REX(1,0,0,0),
                    0x89,
                    MOD_BYTE(0x03, x64_reg[GET_RA(inst)], x64_reg[GET_RD(inst)])
                };
                append_arr_to_vector(tcode, mc, sizeof(mc), 1);
                break;
            }
            case OP_MOVI:
            {
                uint32_t imm = GET_IMM19(inst);
                uint8_t mc[] = {
                    REX(1,0,0,0),
                    0xc7,
                    MOD_BYTE(0x03, 0, x64_reg[GET_RD(inst)]),
                    0,
                    0,
                    0,
                    0
                };
                *(uint32_t*)(mc + 3) = imm;
                append_arr_to_vector(tcode, mc, sizeof(mc), 1);
                break;

            }

            case OP_ADDI:
            {
                uint32_t imm = GET_IMM14(inst);
                uint8_t mc[] = {
                    REX(1, 0, 0, 0),
                    0x81,
                    MOD_BYTE(0x03, 0, x64_reg[GET_RA(inst)]),
                    0,
                    0,
                    0,
                    0,
                    REX(1, 0, 0, 0),
                    0x89,
                    MOD_BYTE(0x03, x64_reg[GET_RA(inst)], x64_reg[GET_RD(inst)])
                };
                *(uint32_t*)(mc + 3) = imm;
                append_arr_to_vector(tcode, mc, sizeof(mc), 1);
                break;
            }
            case OP_SUBI:
            {
                uint32_t imm = GET_IMM19(inst);
                uint8_t mc[] = {
                    REX(1, 0, 0, 0),
                    0x81,
                    MOD_BYTE(0x03, 0x05, x64_reg[GET_RA(inst)]),
                    0,
                    0,
                    0,
                    0,
                    REX(1,0,0,0),
                    0x89,
                    MOD_BYTE(0x03, x64_reg[GET_RA(inst)], x64_reg[GET_RD(inst)])
                };
                *(uint32_t*)(mc + 3) = imm;
                append_arr_to_vector(tcode, mc, sizeof(mc), 1);
                break;
            }
            case OP_ADD:
            {

                uint8_t mc[] = { 
                    0x48,
                    0x01,
                    0x03 << 6 | x64_reg[GET_RA(inst)] << 3 | x64_reg[GET_RB(inst)], 
                    0x48, 
                    0x89, 
                    0x03 << 6 | x64_reg[GET_RB(inst)] << 3 | x64_reg[GET_RD(inst)] 
                }; 
                append_arr_to_vector(tcode, mc, sizeof(mc), 1); 
                break;
            }
            case OP_SUB:
            {

                uint8_t mc[] = { 
                    0x48,
                    0x29,
                    0x03 << 6 | x64_reg[GET_RA(inst)] << 3 | x64_reg[GET_RB(inst)], 
                    0x48, 
                    0x89, 
                    0x03 << 6 | x64_reg[GET_RB(inst)] << 3 | x64_reg[GET_RD(inst)] 
                }; 
                append_arr_to_vector(tcode, mc, sizeof(mc), 1);
                break;
            }
            case OP_MULT:
            {   
                uint8_t mc[] = { 
                    0x48,
                    0x0F,
                    0xAF,
                    0x03 << 6 | x64_reg[GET_RA(inst)] << 3 | x64_reg[GET_RB(inst)], 
                    0x48, 
                    0x89, 
                    0x03 << 6 | x64_reg[GET_RB(inst)] << 3 | x64_reg[GET_RD(inst)] 
                }; 
                append_arr_to_vector(tcode, mc, sizeof(mc), 1);
                break;
            }
            case OP_DIV:
            {
                /* {div rd ra rb}
                 * mov rax, ra
                 * idiv rb
                 * mov rd, rax
                 * */
                uint8_t mc[] = {
                    0x48,
                    0x89,
                    0x03 << 6 | x64_reg[GET_RA(inst)] << 3, // mov rax, ra
                    0x48,
                    0x33,
                    0x03 << 6 | 0x02 << 3 | 0x02, // xor rdx, rdx
                    0x48,
                    0xf7,
                    0x03 << 6 | 0x07 << 3 | x64_reg[GET_RB(inst)], // idiv rb
                    0x48,
                    0x89,
                    0x03 << 6 | x64_reg[GET_RD(inst)] // mov rd, rax
                };

                append_arr_to_vector(tcode, mc, sizeof(mc), 1);
                break;
            }
            case OP_RET:
                APPEND((*tcode), 0xc3, uint8_t);
                break;
            case OP_HALT:
                APPEND((*tcode), 0xf4, uint8_t);
                break;
            case OP_PUSH:
            {
                uint8_t mc[] = {
                    0xff,
                    MOD_BYTE(0x3, 0x6, x64_reg[GET_RD(inst)]),
                };
                append_arr_to_vector(tcode, mc, sizeof(mc), 1);
                break;
            }
            case OP_CALL:
            {
                uint8_t mc[] = {
                    0xff,
                    MOD_BYTE(0x0, 0x2, x64_reg[GET_RD(inst)]),
                };
                append_arr_to_vector(tcode, mc, sizeof(mc), 1);
                break;
            }
            case OP_CMP:
            {
                uint8_t mc[] = {
                    REX(1, 0, 0, 0),
                    0x3b,
                    MOD_BYTE(0x3, x64_reg[GET_RD(inst)], x64_reg[GET_RA(inst)])
                };
                append_arr_to_vector(tcode, mc, sizeof(mc), 1);
                break;

            }

            case OP_LDR:
            {
                /* { ldr rd [ra + disp] }
                 * mov rd, vm->memory
                 * add ra, rd
                 * mov rd, [ra + disp]
                 * */
                int32_t disp = GET_IMM14(inst);
                uint8_t mc[] = {
                    REX(1,0,0,0),
                    0xb8 + x64_reg[GET_RD(inst)],
                    0,0,0,0,0,0,0,0,
                    REX(1,0,0,0),
                    0x03,
                    MOD_BYTE(0x03, x64_reg[GET_RA(inst)], x64_reg[GET_RD(inst)]),
                    REX(1,0,0,0),
                    0x8b,
                    MOD_BYTE(0x2, x64_reg[GET_RD(inst)], x64_reg[GET_RA(inst)]), // 4-byte displacement
                    0,0,0,0
                };
                *(uint64_t*) (mc + 2) = (uint64_t) vm->memory;
                *(int32_t*) (mc + 16) = disp;
                append_arr_to_vector(tcode, mc, sizeof(mc), 1);
                break;

            }
            case OP_STR:
            {
                /* str rd [ra + disp] */
                int32_t disp = GET_IMM14(inst);
                uint64_t phys_addr = (uint64_t) &vm->memory[GET_RA(inst) + disp];
                uint8_t mc[] = {
                    REX(1,0,0,0),
                    0x89,
                    MOD_BYTE(0x0, x64_reg[GET_RD(inst)], 0x5),
                    0x25,
                    0,0,0,0,0,0,0,0
                };
                *(uint64_t*) (mc + 4) = phys_addr;
                append_arr_to_vector(tcode, mc, sizeof(mc), 1);
                break;
            }

            default:
                fprintf(stderr, "Not Implemented\n");
                exit(EXIT_FAILURE);

        }
    }

}
