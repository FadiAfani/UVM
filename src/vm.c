#include "../include/vm.h"
#include <string.h>


void init_vm(struct vm* vm) {
    memset(vm, 0, sizeof(struct vm));
}

void run(struct vm* vm) {
    switch(READ_INST(vm).as_int) {
        case OP_ADD:
        {
            Reg ra = READ_INST(vm).as_int;
            Reg rb = READ_INST(vm).as_int;
            vm->regs[ra].as_int += vm->regs[rb].as_int;
            break;
        }

        case OP_FADD:
        {

            Reg ra = READ_INST(vm).as_int;
            Reg rb = READ_INST(vm).as_int;
            vm->regs[ra].as_double += vm->regs[rb].as_double;
            break;
            
            break;
        }


        case OP_SUB:
        {
            Reg ra = READ_INST(vm).as_int;
            Reg rb = READ_INST(vm).as_int;
            vm->regs[ra].as_int -= vm->regs[rb].as_int;
            break;
        }

        case OP_FSUB:
        {
            Reg ra = READ_INST(vm).as_int;
            Reg rb = READ_INST(vm).as_int;
            vm->regs[ra].as_double -= vm->regs[rb].as_double;
            break;
        }


        case OP_MULT:
        {
            Reg ra = READ_INST(vm).as_int;
            Reg rb = READ_INST(vm).as_int;
            vm->regs[ra].as_int *= vm->regs[rb].as_int;
            break;
        }


        case OP_FMULT:
        {
            Reg ra = READ_INST(vm).as_int;
            Reg rb = READ_INST(vm).as_int;
            vm->regs[ra].as_double *= vm->regs[rb].as_double;
            break;
        }

        case OP_DIV:
        {
            Reg ra = READ_INST(vm).as_int;
            Reg rb = READ_INST(vm).as_int;
            vm->regs[ra].as_int /= vm->regs[rb].as_int;
            break;
        }

        case OP_FDIV:
        {
            Reg ra = READ_INST(vm).as_int;
            Reg rb = READ_INST(vm).as_int;
            vm->regs[ra].as_double /= vm->regs[rb].as_double;
            break;
        }
        case OP_MOV:
        {
            Reg ra = READ_INST(vm).as_int;
            Reg rb = READ_INST(vm).as_int;
            vm->regs[ra] = vm->regs[rb];
            break;
        }

        default: return;
    }
}


