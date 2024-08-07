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


/* MODR/M
 * mod -> 2 bits
 * reg -> 3 bits
 * rm  -> 3 bits
 * */
#define MOD_BYTE(mod, reg, rm) ( mod << 6 | reg << 3 | rm )

/* REX prefix */
#define REX(W,R,X,B) ( 0x40 | W << 3 | R << 2 | X << 1 | B )

void load_vm_reg_into_x64(struct vm* vm, uint cpu_reg, Reg vm_reg) {
    uint8_t mc[] = {
        REX(1,0,0,0),
        0xb8 + cpu_reg,
        0,0,0,0,0,0,0,0
    };
    *(uint64_t*) (mc + 2) = vm->regs[vm_reg].as_u64;
    append_code(vm, mc, sizeof(mc));
}


void gen_x64(struct vm* vm, size_t addr, size_t len) {
    uint32_t inst;
    struct mc_unit* unit;
    ALLOC_UNIT(unit);
    INIT_UNIT((*unit));
    size_t disp = vm->mmem_size;

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

                append_code(vm, mc, sizeof(mc));
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
                append_code(vm, mc, sizeof(mc));
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
                append_code(vm, mc, sizeof(mc));
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
                append_code(vm, mc, sizeof(mc));
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
                append_code(vm, mc, sizeof(mc));
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
                append_code(vm, mc, sizeof(mc));
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
                append_code(vm, mc, sizeof(mc));
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

                append_code(vm, mc, sizeof(mc));
                break;
            }
            case OP_RET:
                append_code(vm, (uint8_t[]){0xc3}, 1);
                break;
            case OP_HALT:
                append_code(vm, (uint8_t[]){0xf4}, 1);
                break;
            case OP_PUSH:
            {
                uint8_t mc[] = {
                    0xff,
                    MOD_BYTE(0x3, 0x6, x64_reg[GET_RD(inst)]),
                };
                append_code(vm, mc, sizeof(mc));
                break;
            }
            case OP_CALL:
            {
                uint8_t mc[] = {
                    0xff,
                    MOD_BYTE(0x0, 0x2, x64_reg[GET_RD(inst)]),
                };
                append_code(vm, mc, sizeof(mc));
                break;
            }
            case OP_CMP:
            {
                uint8_t mc[] = {
                    REX(1, 0, 0, 0),
                    0x3b,
                    MOD_BYTE(0x3, x64_reg[GET_RD(inst)], x64_reg[GET_RA(inst)])
                };
                append_code(vm, mc, sizeof(mc));
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
                append_code(vm, mc, sizeof(mc));
                break;

            }
            case OP_STR:
            {
                /* { str rd [ra + disp] } 
                 * mov rbx, vm->memory
                 * add ra rbx
                 * mov [ra + disp], rd
                 * */
                int32_t disp = GET_IMM14(inst);
                uint8_t mc[] = {
                    REX(1,0,0,0),
                    0xb8 + 0x01,
                    0,0,0,0,0,0,0,0,
                    REX(1,0,0,0),
                    0x03,
                    MOD_BYTE(0x03, x64_reg[GET_RA(inst)], 0x01),
                    REX(1,0,0,0),
                    0x89,
                    MOD_BYTE(0x2, x64_reg[GET_RD(inst)], x64_reg[GET_RA(inst)]), // 4-byte displacement
                    0,0,0,0
                };
                *(uint64_t*) (mc + 2) = (uint64_t) vm->memory;
                *(int32_t*) (mc + 16) = disp;
                append_code(vm, mc, sizeof(mc));
                break;
            }
            case OP_JB:
            {
                /* { jb label }
                 * label:
                 *      movi r1 0
                 *      cmp r1 r0
                 *      jb label
                 * mov rbx, 0
                 * cmp rbx, rax
                 * jb <label>
                 * */
                uint32_t label = GET_IMM24(inst);

                
                uint8_t mc[] = {
                    0x0f,
                    0x87,
                    0,0,0,0
                };

                *(uint32_t*) (mc + 2) = (uint64_t) (vm->mmem + label);
                break;
            }
            case OP_JBE:
                break;
            case OP_JL:
                break;
            case OP_JLE:
                break;
            case OP_JE:
                break;
            case OP_JNE:
                break;
            case OP_JMP:
                break;

            default:
                fprintf(stderr, "Not Implemented\n");
                exit(EXIT_FAILURE);

        }
    }

    insert_unit_front(vm, unit);

}
