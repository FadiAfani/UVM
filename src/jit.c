#include "../include/vm.h"
#include "../lib/vector.h"
#include "../include/jit.h"
#include <stdlib.h>
#include <stdio.h>

unsigned char x64_reg[] = {
    [R0] = 0x0,
    [R1] = 0x3,
    [R2] = 0x1,
    [R3] = 0x2,
    [R4] = 0x6,
    [R5] = 0x7,
    [R6] = 0x5,
    [R7] = 0x4,

};



Vector* gen_x64(struct vm* vm, size_t addr, size_t len) {
    Vector* tcode;
    ALLOC_VECTOR(tcode);
    INIT_VECTOR((*tcode), 1); // vector<char>
    uint32_t inst;

    //TODO: fail when addr/len > MEM_SIZE

    for (size_t i = 0; i < len; i++) {
        inst = vm->memory[addr + i];
        uint8_t opcode = GET_OPCODE(inst);
        switch(opcode) {
            case OP_MOVI:
            {
                uint32_t imm = GET_IMM19(inst);
                unsigned char x64_code[] = {
                    0xb8 + x64_reg[GET_RD(inst)],
                    0,
                    0,
                    0,
                    0
                };

                memcpy(x64_code + 1, &imm, 4);
                append(tcode, x64_code, sizeof(x64_code), 1);
                break;

            }
            case OP_ADD:
            {
                /* add and mov
                 * <prefix>
                 * <add-opcode>
                 * <mod + regs>
                 * <prefix> 
                 * <mov-opcode>
                 * <mod + regs>
                */
                unsigned char x64_code[] = {
                    0x48,
                    0x01,
                    0x03 << 6 | x64_reg[GET_RA(inst)] << 3 | x64_reg[GET_RB(inst)],
                    0x48,
                    0x89,
                    0x03 << 6 | x64_reg[GET_RB(inst)] << 3 | x64_reg[GET_RD(inst)],
                };


                append(tcode, x64_code, sizeof(x64_code), 1);
                break;
            }
            case OP_RET:
                append(tcode, (unsigned char[]) {0xc3}, 1, 1);
                break;

            case OP_HALT:
                return tcode;
            default:
                fprintf(stderr, "Not Implemented\n");
                exit(EXIT_FAILURE);

        }
    }

    return tcode;
}
