#include "../include/vm.h"
#include <string.h>
#include <stdio.h>

void init_vm(struct vm* vm) {
    memset(vm, 0, sizeof(struct vm));
    vm->regs[RSP].as_u32 = STACK_SIZE + 1;
}

void run(struct vm* vm) {
    uint32_t inst = READ_INST(vm);
    uint8_t opcode = (inst & OPCODE_MASK) >> 24;
    printf("opcode: %d\n", opcode);

    switch(opcode) {
        case OP_ADD:
            BIN_OP_REG(vm, inst, as_int, +);
            break;

        case OP_ADDI:
            printf("rip: %d\n", vm->regs[RIP].as_u32);
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
            vm->regs[ra] = vm->regs[ra];
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
            vm->regs[rd] = vm->memory[ vm->regs[RSP].as_u32 ];
            vm->regs[RSP].as_u32++;
            break;
        }

        case OP_CALL:
        {
            uint32_t label = GET_IMM24(inst);
            STACK_PUSH(vm, vm->regs[RSP]);
            vm->regs[RIP].as_u32 = label;
            break;
        }
        
        default: return;
    }
}


