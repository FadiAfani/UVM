#include "../include/vm.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

void init_vm(struct vm* vm) {

    if (vm != NULL) {
        memset(vm, 0, sizeof(struct vm));
        vm->regs[R7].as_u32 = MEM_SIZE;
        vm->regs[R6].as_u32 = MEM_SIZE;
        vm->mmem = reinterpret_cast<uint8_t*>(mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0));
        vm->mmem_cap = 4096;
        vm->tp = NULL;
        vm->is_tracing = true;
        vm->native_exec = true;
    }

     
}


static inline int interpret(struct vm* vm, uint32_t inst) {
    uint8_t opcode = GET_OPCODE(inst);

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
            unsigned int rd = GET_RD(inst);
            unsigned int ra = GET_RA(inst);
            vm->regs[rd] = vm->regs[ra];
            break;
        }
        case OP_CMP:
        {
            unsigned int rd = GET_RD(inst);
            unsigned int ra = GET_RA(inst);
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
            unsigned int rd = GET_RD(inst);
            unsigned int ra = GET_RA(inst);
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
            unsigned int rd = GET_RD(inst);
            vm->regs[rd].as_u32 = vm->memory[ vm->regs[R7].as_u32++ ];
            break;
        }

        case OP_PUSH:
        {
            unsigned int rd = GET_RD(inst);
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
            unsigned int rd = GET_RD(inst);
            uint16_t imm = GET_IMM14(inst);
            vm->regs[rd].as_u32 = imm;
            break;

        }
        case OP_LDR:
        {
            unsigned int rd = GET_RD(inst);
            unsigned int rs = GET_RA(inst);
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
            unsigned int rd = GET_RD(inst);
            unsigned int rs = GET_RA(inst);
            uint16_t imm = GET_IMM14(inst);
            int msb = (imm & ( 1 << 13 )) >> 13;
            int16_t shift = (( 1 << 13 ) - 1) & imm;
            if (msb) {
                shift = -shift;
            }
            vm->memory[vm->regs[rs].as_u32 + shift] = vm->regs[rd].as_u32;
            break;
        }


        case OP_RET:
            if (vm->regs[R6].as_u32 < MEM_SIZE) {
                vm->regs[RIP].as_u32 = vm->memory[vm->regs[R6].as_u32 - 1];
            } else {
                return 0;
            }
            break;
        case OP_HALT: return 0;
        
        default: 
            printf("unrecognized opcode: %d\n", opcode);
            exit(EXIT_FAILURE);
    }
    return 1;
}

static inline void profile(struct vm* vm, uint32_t prev_ip) {
        uint32_t ip = GET_REG_AS(RIP, as_u32);
        LoopHeader lh;
        if (vm == NULL)
            return;
        if (ip < prev_ip) {
            try {
                lh = vm->loop_headers.at(ip);
                lh.heat++;
                vm->is_tracing = false;
            } catch (std::exception e) {
                lh.heat = 0;
                lh.trace = NULL;
                vm->loop_headers[ip] = lh;
                vm->is_tracing = true;

            }
            vm->tp = lh.trace;

        }
        
}

static inline void record_inst(struct vm* vm, uint32_t inst) {
    Trace* trace = vm->tp;
    if (vm == NULL || trace == NULL || !vm->is_tracing) 
        return;

    uint32_t jmp_addr = GET_IMM24(inst);
    uint32_t prev_ip = GET_REG_AS(RIP, as_u32);
    uint32_t cur_ip;
    switch(GET_OPCODE(inst)) {
        case OP_JB:
        case OP_JE:
        case OP_JL:
        case OP_JBE:
        case OP_JLE:
        case OP_JNE:
        case OP_JMP:
            interpret(vm, inst);
            break;
        
        default:
            APPEND(trace->bytecode, inst, uint32_t);
            break;
    }

    
}

void run(struct vm* vm) {
    if (vm == NULL) 
        return;

    uint32_t ip = 0;

    for (;;) {
        uint32_t inst = READ_INST(vm);

        profile(vm, ip);
        record_inst(vm, inst);

        if (vm->native_exec) {
            int (*fptr)() = (void*) vm->tp->mmem_ptr;
        } else {
            int interp_res = interpret(vm, inst);
            if (!interp_res) return;
        }
        ip = GET_REG_AS(RIP, as_u32);

    }
}

void write_trace(struct vm* vm, Trace* trace) {
    if (trace == NULL || vm == NULL)
        return;

    //TODO: mmem is not dynamic !!! 
    trace->mmem_ptr = vm->mmem + vm->mmem_size;
    gen_x64(vm, &trace->bytecode);

}

