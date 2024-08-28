#include "../include/jit.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define UNMAPPED vm->mmem_cap

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
#define MOD_BYTE(mod, reg, rm) ( (uint8_t) (mod << 6 | reg << 3 | rm ) )

/* REX prefix */
#define REX(W,R,X,B) ( 0x40 | W << 3 | R << 2 | X << 1 | B )


static int append_code(struct vm* vm, uint8_t* code, size_t len) {
    if (vm == NULL)
        return -1;
    if (code != NULL) {
        memcpy(vm->mmem + vm->mmem_size, code, len);
        vm->mmem_size += len;
    }

    return 0;
}

void dump_output_into_file(const char *fn, uint8_t *buff, size_t len) {
    FILE* fd;
    if (fn == NULL)
        fd = fopen("dump", "wb");
    else 
        fd = fopen(fn, "wb");

    if (buff != NULL) 
        fwrite(buff, 1, len, fd);
    
    fclose(fd);

}


void transfer_reg_x64(struct vm* vm, unsigned int cpu_reg, Reg vm_reg, bool to_cpu) {
    uint8_t opcode = 0x89;
    if (to_cpu) {
        opcode = 0x8b;
    }

    /* mov rax, imm64
     * mov cpu_reg, [rax]
     * */
    uint8_t mc[] = {
        REX(1,0,0,0),
        0xb8,
        0,0,0,0,0,0,0,0,
        REX(1,0,0,0),
        opcode,
        MOD_BYTE(0, cpu_reg, 0)
    };
    *(uint64_t*) (mc + 2) = GET_REG_AS(vm_reg, as_u64);
    append_code(vm, mc, sizeof(mc));

}

void transfer_reg_state(struct vm* vm, ArchType arch, bool to_cpu) {
    RegTransferFunc tfunc;
    uint8_t* cpu_regs;
    int n;
    switch(arch) {
        case X86_64:
            tfunc = transfer_reg_x64;
            cpu_regs = x64_reg;
            n = sizeof(x64_reg);
            break;
        default:
            fprintf(stderr, "not a valid register transfer function\n");
            exit(EXIT_FAILURE);
    }

    for (int i = 0; i < n; i++) { 
        tfunc(vm, cpu_regs[i], (Reg) i, to_cpu);
    }

}


void gen_x64(struct vm* vm, Vector* bytecode) {
    uint32_t inst;
    HashTable jmp_pts;
    init_hash_table(&jmp_pts);

    //TODO: fail when addr/len > MEM_SIZE
    for (size_t i = 0; i < bytecode->size; i++) {
        inst = INDEX_VECTOR((*bytecode), uint32_t, i);
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
                uint8_t mod1 = 0x03 << 6 | x64_reg[GET_RA(inst)] << 3 | x64_reg[GET_RB(inst)];
                uint8_t mod2 = 0x03 << 6 | x64_reg[GET_RB(inst)] << 3 | x64_reg[GET_RD(inst)];

                uint8_t mc[] = { 
                    0x48,
                    0x01,
                    mod1,
                    0x48, 
                    0x89, 
                    mod2
                }; 
                append_code(vm, mc, sizeof(mc));
                break;
            }
            case OP_SUB:
            {

                uint8_t mod1 = 0x03 << 6 | x64_reg[GET_RA(inst)] << 3 | x64_reg[GET_RB(inst)];
                uint8_t mod2 = 0x03 << 6 | x64_reg[GET_RB(inst)] << 3 | x64_reg[GET_RD(inst)];

                uint8_t mc[] = { 
                    0x48,
                    0x29,
                    mod1,
                    0x48, 
                    0x89, 
                    mod2
                }; 
                append_code(vm, mc, sizeof(mc));
                break;
            }
            case OP_MULT:
            {   

                uint8_t mod1 = 0x03 << 6 | x64_reg[GET_RA(inst)] << 3 | x64_reg[GET_RB(inst)];
                uint8_t mod2 = 0x03 << 6 | x64_reg[GET_RB(inst)] << 3 | x64_reg[GET_RD(inst)];

                uint8_t mc[] = { 
                    0x48,
                    0x0F,
                    0xAF,
                    mod1,
                    0x48, 
                    0x89, 
                    mod2
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

                uint8_t mod1 = 0x03 << 6 | x64_reg[GET_RA(inst)] << 3; // mov rax, ra
                uint8_t mod2 = 0x03 << 6 | 0x07 << 3 | x64_reg[GET_RB(inst)]; // idiv rb
                uint8_t mod3 = 0x03 << 6 | x64_reg[GET_RD(inst)]; // mov rd, rax
                                                                  //
                uint8_t mc[] = {
                    0x48,
                    0x89,
                    mod1,
                    0x48,
                    0x33,
                    0x03 << 6 | 0x02 << 3 | 0x02, // xor rdx, rdx
                    0x48,
                    0xf7,
                    mod2,
                    0x48,
                    0x89,
                    mod3
                };

                append_code(vm, mc, sizeof(mc));
                break;
            }
            case OP_RET:
            {
                uint8_t mc[] = {0xc3};
                append_code(vm, mc, 1);
                break;
            }
            case OP_HALT:
            {
                uint8_t mc[] = {0xf4};
                append_code(vm, mc, 1);
                break;
            }
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
                uint8_t dst = 0xb8 + x64_reg[GET_RD(inst)];
                int32_t disp = GET_IMM14(inst);
                uint8_t mc[] = {
                    REX(1,0,0,0),
                    dst,
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
                break;
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
    //free_hash_table(&jmp_pts);
}
