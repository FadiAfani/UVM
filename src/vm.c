#include "../include/vm.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

void init_vm(struct vm* vm) {

    if (vm != NULL) {
        memset(vm, 0, sizeof(struct vm));
        vm->regs[R7].as_u32 = MEM_SIZE;
        vm->regs[R6].as_u32 = MEM_SIZE;
    }

     
}

void run(struct vm* vm) {
    for (;;) {

        uint32_t inst = READ_INST(vm);
        uint8_t opcode = GET_OPCODE(inst);
        //printf("rfp: %d, opcode: %d\n", vm->regs[RIP].as_u32, opcode);

        switch(opcode) {
            case OP_ADD:
                BIN_OP_REG(vm, inst, as_int, +);
                break;

            case OP_ADDI:
                BIN_OP_IMM(vm, inst, as_int, +);
                break;

            case OP_FADD:
                BIN_OP_REG(vm, inst, as_double, +);
                break;

            case OP_SUB:
                BIN_OP_REG(vm, inst, as_int, -);
                break;

            case OP_SUBI:
                BIN_OP_IMM(vm, inst, as_int, -);
                break;

            case OP_FSUB:
                BIN_OP_REG(vm, inst, as_double, -);
                break;

            case OP_MULT:
                BIN_OP_REG(vm, inst, as_int, *);
                break;

            case OP_FMULT:
                BIN_OP_REG(vm, inst, as_double, *);
                break;

            case OP_DIV:
                BIN_OP_REG(vm, inst, as_int, /);
                break;

            case OP_FDIV:
                BIN_OP_REG(vm, inst, as_double, /);
                break;
            case OP_MOV:
            {
                Reg rd = GET_RD(inst);
                Reg ra = GET_RA(inst);
                vm->regs[rd] = vm->regs[ra];
                break;
            }
            case OP_CMP:
            {
                Reg rd = GET_RD(inst);
                Reg ra = GET_RA(inst);
                if (vm->regs[rd].as_int > vm->regs[ra].as_int) 
                    vm->regs[RFLG].as_int = 1;
                else if (vm->regs[rd].as_int < vm->regs[ra].as_int)
                    vm->regs[RFLG].as_int = -1;
                else 
                    vm->regs[RFLG].as_int = 0;
                break;
            }

            case OP_FCMP:
            {
                Reg rd = GET_RD(inst);
                Reg ra = GET_RA(inst);
                if (vm->regs[rd].as_double > vm->regs[ra].as_double) 
                    vm->regs[RFLG].as_int = 1;
                else if (vm->regs[rd].as_int < vm->regs[ra].as_int)
                    vm->regs[RFLG].as_int = -1;
                else 
                    vm->regs[RFLG].as_int = 0;
                break;
            }

            case OP_JMP:
            {
                uint32_t label = GET_IMM24(inst);
                vm->regs[RIP].as_u32 = label;
                break;
            }
            case OP_JE:
            {
                uint32_t label = GET_IMM24(inst);
                if (vm->regs[RFLG].as_int == 0)
                    vm->regs[RIP].as_u32 = label;
                break;
            }

            case OP_JL:
            {
                uint32_t label = GET_IMM24(inst);
                if (vm->regs[RFLG].as_int < 0)
                    vm->regs[RIP].as_u32 = label;
                break;
            }


            case OP_JB:
            {
                uint32_t label = GET_IMM24(inst);
                if (vm->regs[RFLG].as_int > 0)
                    vm->regs[RIP].as_u32 = label;
                break;
            }

            case OP_POP: 
            {
                Reg rd = GET_RD(inst);
                vm->regs[rd].as_u32 = vm->memory[ vm->regs[R7].as_u32++ ];
                break;
            }

            case OP_PUSH:
            {
                Reg rd = GET_RD(inst);
                STACK_PUSH(vm, vm->regs[rd].as_u32);
                break;
            }

            case OP_CALL:
            {
                uint32_t label = GET_IMM24(inst);
                vm->memory[ vm->regs[R6].as_u32 - 1 ] = vm->regs[RIP].as_u32;
                vm->regs[RIP].as_u32 = label;
                break;
            }
            case OP_JBE:
            {
                uint32_t label = GET_IMM24(inst);
                if (vm->regs[RFLG].as_int >= 0 )
                    vm->regs[RIP].as_u32 = label;
                break;
            }

            case OP_JLE:
            {
                uint32_t label = GET_IMM24(inst);
                if (vm->regs[RFLG].as_int <= 0 )
                    vm->regs[RIP].as_u32 = label;
                break;
            }

            case OP_JNE:
            {
                uint32_t label = GET_IMM24(inst);
                if (vm->regs[RFLG].as_int != 0 )
                    vm->regs[RIP].as_u32 = label;
                break;
            }
            case OP_MOVI:
            {
                Reg rd = GET_RD(inst);
                uint16_t imm = GET_IMM14(inst);
                vm->regs[rd].as_u32 = imm;
                break;

            }
            case OP_LDR:
            {
                Reg rd = GET_RD(inst);
                Reg rs = GET_RA(inst);
                uint16_t imm = GET_IMM14(inst);
                int msb = (imm & ( 1 << 13 )) >> 13;
                int16_t shift = (( 1 << 13 ) - 1) & imm;
                if (msb) {
                    shift = -shift;
                }
                vm->regs[rd].as_u32 = vm->memory[vm->regs[rs].as_u32 + shift];
                break;

            }
            
            case OP_STR:
            {
                Reg rd = GET_RD(inst);
                Reg rs = GET_RA(inst);
                uint16_t imm = GET_IMM14(inst);
                int msb = (imm & ( 1 << 13 )) >> 13;
                int16_t shift = (( 1 << 13 ) - 1) & imm;
                if (msb) {
                    shift = -shift;
                }
                //printf("mem: %d, %d\n", vm->regs[rs].as_u32, shift);
                vm->memory[vm->regs[rs].as_u32 + shift] = vm->regs[rd].as_u32;
                break;
            }


            case OP_RET:
                if (vm->regs[R6].as_u32 < MEM_SIZE) {
                    vm->regs[RIP].as_u32 = vm->memory[vm->regs[R6].as_u32 - 1];
                } else {
                    return;
                }
                //printf("ret: %d\n", vm->regs[RIP].as_u32);
                break;
            case OP_HALT: return;
            
            default: 
                //printf("opcode: %d\n", vm->regs[RIP].as_u32);
                printf("unrecognized opcode: %d\n", opcode);
                exit(EXIT_FAILURE);
        }

    }
}


