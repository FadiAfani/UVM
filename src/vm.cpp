#include <cstring>
#include <fstream>
#include <stdexcept>
#include <stdlib.h>
#include <sys/mman.h>
#include <assert.h>
#include "../include/jit.h"

VM::VM(bool jit_enabled) {
    std::memset(this->regs, 0, sizeof(this->regs));
    this->regs[R7].as_int = -1;
    this->regs[R6].as_int = -1;
    if (jit_enabled) {
        this->jit = new JITCompiler();
    }
    
}

Word& VM::get_reg(Reg r) {
    return this->regs[r];
}

uint32_t VM::fetch() {
    return this->memory[ this->regs[RIP].as_u32++ ];
}

uint8_t VM::decode(uint32_t inst) {
    return (inst & OPCODE_MASK) >> 24;
}

void VM::set_memory_addr(uint32_t addr, uint32_t value) {
    this->memory[addr] = value;
}

uint32_t VM::read_memory(uint32_t addr) {
    return this->memory[addr];
}

int VM::interpret(uint32_t inst) {
    uint8_t opcode = this->decode(inst);

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

    if (fn == NULL) {
        throw std::runtime_error("specify a file path - fn is null");
    }

    std::fstream fd;
    fd.open(fn, std::fstream::out | std::fstream::in | std::fstream::binary);
    fd.seekg(0, std::fstream::end);
    size_t len = fd.tellg();
    fd.seekg(0, std::fstream::beg);
    char* buff = reinterpret_cast<char*>(this->memory);
    fd.read(buff, len);
    fd.close();
}


void VM::save_mod_regs(uint32_t inst) {
    if (this->jit == nullptr)
        throw std::runtime_error("jit mode is not enabled");
    else if (this->jit->peek_top_trace() == nullptr)
        return;
    Reg rd;
    switch(this->decode(inst)) {
        /* instructions that use the rd register */
        case OP_ADD:
        case OP_ADDI:
        case OP_FADD:
        case OP_SUB:
        case OP_SUBI:
        case OP_FSUB:
        case OP_MOV:
        case OP_MOVI:
        case OP_MULT:
        case OP_FMULT:
        case OP_DIV:
        case OP_FDIV:
        case OP_CMP:
        case OP_FCMP:
        case OP_STR:
            rd = GET_RD(inst);
            //this->jit->get_active_trace()->mod_regs.push(rd);
            break;
    }

}

void VM::run() {

    if (this->jit == nullptr) {
        for (;;) {
            uint32_t inst = this->fetch();
            int interp_res = this->interpret(inst);
            if (!interp_res) 
                return;
        }
    }

    for (;;) {

        uint32_t inst = this->fetch();
        this->jit->profile(this, inst);
        this->jit->record_inst(this, inst);


        if (!this->jit->get_is_tracing()) {
            Trace* tp = this->jit->pop_trace();
            if (tp != nullptr && tp->get_func() == nullptr) {
                this->jit->transfer_reg_state(this, true, transfer_reg_x64);
                this->jit->compile_trace(this, tp);
                this->jit->transfer_reg_state(this, false, transfer_reg_x64);
            }


            if (tp != nullptr) {
                /* transfer vm state to cpu */
                tp->get_func()();
                /* transfer cpu state to vm */
            }
            

        } 

        int interp_res = this->interpret(inst);
        if (!interp_res) return;

        

    }

}


