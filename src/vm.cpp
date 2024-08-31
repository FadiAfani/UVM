#include <fstream>
#include <stdlib.h>
#include <sys/mman.h>
#include "../include/jit.h"

VM::VM() {}

Word VM::get_reg(Reg r) {
    return this->regs[r];
}

int VM::interpret(uint32_t inst) {
    uint8_t opcode = GET_OPCODE(inst);

    switch(opcode) {
        case OP_ADD:
            BIN_OP_REG(this, inst, as_int, +);
            break;

        case OP_ADDI:
            BIN_OP_IMM(this, inst, as_int, +);
            break;

        case OP_FADD:
            BIN_OP_REG(this, inst, as_double, +);
            break;

        case OP_SUB:
            BIN_OP_REG(this, inst, as_int, -);
            break;

        case OP_SUBI:
            BIN_OP_IMM(this, inst, as_int, -);
            break;

        case OP_FSUB:
            BIN_OP_REG(this, inst, as_double, -);
            break;

        case OP_MULT:
            BIN_OP_REG(this, inst, as_int, *);
            break;

        case OP_FMULT:
            BIN_OP_REG(this, inst, as_double, *);
            break;

        case OP_DIV:
            BIN_OP_REG(this, inst, as_int, /);
            break;

        case OP_FDIV:
            BIN_OP_REG(this, inst, as_double, /);
            break;
        case OP_MOV:
        {
            Reg rd = GET_RD(inst);
            Reg ra = GET_RA(inst);
            this->regs[rd] = this->regs[ra];
            break;
        }
        case OP_CMP:
        {
            Reg rd = GET_RD(inst);
            Reg ra = GET_RA(inst);
            if (this->regs[rd].as_int > this->regs[ra].as_int) 
                this->regs[RFLG].as_int = 1;
            else if (this->regs[rd].as_int < this->regs[ra].as_int)
                this->regs[RFLG].as_int = -1;
            else 
                this->regs[RFLG].as_int = 0;
            break;
        }

        case OP_FCMP:
        {
            Reg rd = GET_RD(inst);
            Reg ra = GET_RA(inst);
            if (this->regs[rd].as_double > this->regs[ra].as_double) 
                this->regs[RFLG].as_int = 1;
            else if (this->regs[rd].as_int < this->regs[ra].as_int)
                this->regs[RFLG].as_int = -1;
            else 
                this->regs[RFLG].as_int = 0;
            break;
        }

        case OP_JMP:
        {
            uint32_t label = GET_IMM24(inst);
            this->regs[RIP].as_u32 = label;
            break;
        }
        case OP_JE:
        {
            uint32_t label = GET_IMM24(inst);
            if (this->regs[RFLG].as_int == 0)
                this->regs[RIP].as_u32 = label;
            break;
        }

        case OP_JL:
        {
            uint32_t label = GET_IMM24(inst);
            if (this->regs[RFLG].as_int < 0)
                this->regs[RIP].as_u32 = label;
            break;
        }


        case OP_JB:
        {
            uint32_t label = GET_IMM24(inst);
            if (this->regs[RFLG].as_int > 0)
                this->regs[RIP].as_u32 = label;
            break;
        }

        case OP_POP: 
        {
            Reg rd = GET_RD(inst);
            this->regs[rd].as_u32 = this->memory[ this->regs[R7].as_u32++ ];
            break;
        }

        case OP_PUSH:
        {
            Reg rd = GET_RD(inst);
            STACK_PUSH(this, this->regs[rd].as_u32);
            break;
        }

        case OP_CALL:
        {
            uint32_t label = GET_IMM24(inst);
            this->memory[ this->regs[R6].as_u32 - 1 ] = this->regs[RIP].as_u32;
            this->regs[RIP].as_u32 = label;
            break;
        }
        case OP_JBE:
        {
            uint32_t label = GET_IMM24(inst);
            if (this->regs[RFLG].as_int >= 0 )
                this->regs[RIP].as_u32 = label;
            break;
        }

        case OP_JLE:
        {
            uint32_t label = GET_IMM24(inst);
            if (this->regs[RFLG].as_int <= 0 )
                this->regs[RIP].as_u32 = label;
            break;
        }

        case OP_JNE:
        {
            uint32_t label = GET_IMM24(inst);
            if (this->regs[RFLG].as_int != 0 )
                this->regs[RIP].as_u32 = label;
            break;
        }
        case OP_MOVI:
        {
            Reg rd = GET_RD(inst);
            uint16_t imm = GET_IMM14(inst);
            this->regs[rd].as_u32 = imm;
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
            this->regs[rd].as_u32 = this->memory[this->regs[rs].as_u32 + shift];
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
            this->memory[this->regs[rs].as_u32 + shift] = this->regs[rd].as_u32;
            break;
        }


        case OP_RET:
            if (this->regs[R6].as_u32 < MEM_SIZE) {
                this->regs[RIP].as_u32 = this->memory[this->regs[R6].as_u32 - 1];
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

void VM::load_binary_file(const char* fn) {
    std::fstream fd(fn, std::ios::binary);
    size_t len = fd.tellg();
    fd.seekg(0, std::ios::beg);
    char* buff = reinterpret_cast<char*>(this->memory);
    fd.read(buff, len);
}

void VM::run() {
    Trace* tp;
    for (;;) {
        uint32_t inst = READ_INST(this);
        this->jit->profile(this, inst);
        this->jit->record_inst(this, inst);
        //this->jit->get_mod_regs(inst);
        tp = this->jit->get_tp();

        if (this->jit->get_native_exec() && tp != nullptr && !this->jit->get_is_tracing()) {
            /* trace needs to be compiled */
            //tp->func();

        } else {
            int interp_res = this->interpret(inst);
            if (!interp_res) return;
        }

    }

}


