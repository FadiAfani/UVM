#include <cstring>
#include <fstream>
#include <stdexcept>
#include <stdlib.h>
#include <sys/mman.h>
#include <assert.h>
#include "../include/vm.h"

VM::VM() {
    std::memset(this->regs, 0, sizeof(this->regs));
    this->regs[R7].as_int = -1;
    this->regs[R6].as_int = -1;
    
}

void VM::set_reg(std::stack<Reg>& mod_regs, Word val, Reg r) {
    mod_regs.push(r);
    this->regs[r] = val;
}

Word& VM::get_reg(Reg r) {
    return this->regs[r];
}

Word* VM::get_reg_array() {
    return this->regs;
}

Word* VM::get_reg_as_ref(Reg r) {
    return &this->regs[r];
}


const uint32_t* VM::get_memory() { return this->memory; }

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
            else if (this->regs[rd].as_double < this->regs[ra].as_double)
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

void VM::run() {
    for (;;) {
        uint32_t inst = this->fetch();
        int res = this->interpret(inst);
        if (!res)
            return;
    }
}

UVMAssembler::UVMAssembler(VM& vm) : vm(vm) {};

void inline UVMAssembler::write_inst(uint8_t op, uint8_t rd, uint8_t ra, uint8_t rb) {
    vm.set_memory_addr(this->pos++, op << 24 | rd << 19 | ra << 14 | rb << 9);
}


void inline UVMAssembler::write_inst(uint8_t op, uint8_t rd, uint8_t ra, uint16_t imm) {
    vm.set_memory_addr(this->pos++, op << 24 | rd << 19 | ra << 14 | imm);
}

void inline UVMAssembler::write_inst(uint8_t op, uint8_t rd, uint8_t ra) {
    vm.set_memory_addr(this->pos++, op << 24 | rd << 19 | ra << 14);
}

void inline UVMAssembler::write_inst(uint8_t op, uint8_t rd) {
    vm.set_memory_addr(this->pos++, op << 24 | rd << 19);
}

void inline UVMAssembler::write_inst(uint8_t op, uint32_t label) {
    vm.set_memory_addr(this->pos++, op << 24 | label);
}

void UVMAssembler::add(uint8_t rd, uint8_t ra, uint8_t rb) {
    write_inst(OP_ADD, rd, ra, rb);
}

void UVMAssembler::addi(uint8_t rd, uint8_t ra, uint16_t imm) {
    write_inst(OP_ADDI, rd, ra, imm);
}

void UVMAssembler::fadd(uint8_t rd, uint8_t ra, uint8_t rb) {
    write_inst(OP_FADD, rd, ra, rb);
}

void UVMAssembler::sub(uint8_t rd, uint8_t ra, uint8_t rb) {
    write_inst(OP_SUB, rd, ra, rb);
}

void UVMAssembler::subi(uint8_t rd, uint8_t ra, uint16_t imm) {
    write_inst(OP_SUBI, rd, ra, imm);
}

void UVMAssembler::mult(uint8_t rd, uint8_t ra, uint8_t rb) {
    write_inst(OP_MULT, rd, ra, rb);
}

void UVMAssembler::div(uint8_t rd, uint8_t ra) {
    write_inst(OP_DIV, rd, ra);
}

void UVMAssembler::mov(uint8_t rd, uint8_t ra) {
    write_inst(OP_MOV, rd, ra);
}

void UVMAssembler::call(uint32_t label) {
    write_inst(OP_CALL, label);
}

void UVMAssembler::push(uint8_t rd) {
    write_inst(OP_PUSH, rd);
}

void UVMAssembler::pop(uint8_t rd) {
    write_inst(OP_POP, rd);
}

void UVMAssembler::ret() {
    vm.set_memory_addr(this->pos++, OP_RET << 24);
}

void UVMAssembler::cmp(uint8_t rd, uint8_t ra) {
    write_inst(OP_CMP, rd, ra);
}

void UVMAssembler::jmp(uint32_t label) {
    write_inst(OP_JMP, label);
}

void UVMAssembler::jb(uint32_t label) {
    write_inst(OP_JB, label);
}

void UVMAssembler::je(uint32_t label) {
    write_inst(OP_JE, label);
}

void UVMAssembler::jl(uint32_t label) {
    write_inst(OP_JL, label);
}

void UVMAssembler::jle(uint32_t label) {
    write_inst(OP_JLE, label);
}

void UVMAssembler::jbe(uint32_t label) {
    write_inst(OP_JBE, label);
}

void UVMAssembler::jne(uint32_t label) {
    write_inst(OP_JNE, label);
}

void UVMAssembler::halt() {
    vm.set_memory_addr(this->pos++, OP_HALT << 24);
}

void UVMAssembler::movi(uint8_t rd, uint32_t imm) {
    vm.set_memory_addr(this->pos++, OP_MOVI << 24 | rd << 19 | imm);

}

