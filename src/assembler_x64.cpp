#include "../include/assembler_x64.h"

using namespace X64;

#define OP_MASK 0xff
#define IMM_OP_MASK 0xffffff << 8

void Assembler::emit_inst_rr(std::initializer_list<uint8_t> op, Register dst, Register src) {
    if (dst.size > 8 || src.size > 8 || dst.size != src.size)
        throw std::logic_error("malformed instruction: register/register"); 
    EMIT_REX(dst, REX(1,0,0,0) );
    for (auto x : op)
        this->emit_byte(x);
    this->emit_byte( MOD_BYTE(3, dst.encoding, src.encoding) );
}

void Assembler::emit_inst_rm(std::initializer_list<uint8_t> op, Register dst, MemOp src) {
    if (src.disp_size > 4 || dst.size > 8 || dst.size != src.reg.size)
        throw std::logic_error("malformed instruction: register/memory");
    for (auto x : op)
        this->emit_byte(x);
    this->emit_byte( MOD_BYTE(src.disp_size == 4 ? 3 : src.disp_size, dst.encoding, src.reg.encoding));
    switch(src.disp_size) { 
        case 0: 
        case 1: 
            this->emit_imm(src.disp.as_i8);
            break; 
        case 2: 
            this->emit_imm(src.disp.as_i16); 
            break; 
        case 4: 
            this->emit_imm(src.disp.as_i32); 
            break; 
    } 
};

template<typename T>
void Assembler::emit_inst_ri(std::initializer_list<uint8_t> op, uint8_t opex, Register dst, T imm) {
    if (dst.size > 8 || sizeof(imm) > 8 || dst.size != sizeof(imm))
        throw std::logic_error("malformed instruction: register/immediate");

    EMIT_REX(dst, REX(1,0,0,0));
    for (auto x : op)
        this->emit_byte(x);
    this->emit_byte( MOD_BYTE(3, opex, dst.encoding) );
    this->emit_imm(imm);
}

template<typename T>
void Assembler::emit_inst_mi(std::initializer_list<uint8_t> op, uint8_t opex, MemOp dst, T imm) {
    if (sizeof(imm) > 8 || dst.disp_size > 4 || dst.reg.size > 8)
        throw std::logic_error("malformed instruction: memory/immediate");

    EMIT_REX(dst.reg, REX(1,0,0,0));
    for (auto x : op)
        this->emit_byte(x);
    this->emit_byte( MOD_BYTE(dst.disp_size == 4 ? 3 : dst.disp_size, opex, dst.reg.encoding));
    switch(dst.disp_size) { 
        case 0: 
        case 1: 
            this->emit_imm(dst.disp.as_i8);
            break; 
        case 2: 
            this->emit_imm(dst.disp.as_i16); 
            break; 
        case 4: 
            this->emit_imm(dst.disp.as_i32); 
            break; 
    } 
    this->emit_imm(imm);

}

void Assembler::mov(Register dst, Register src) {
    this->emit_inst_rr({0x89}, dst, src);
}

void Assembler::mov(Register dst, MemOp src) {
    this->emit_inst_rm({0x8b}, dst, src);
}

void Assembler::mov(MemOp dst, Register src) {
    this->emit_inst_rm({0x89}, src, dst);
}

template<typename T>
void Assembler::mov(Register dst, T imm) {
    uint8_t op = 0xb8;
    if (sizeof(T) == 1)
        op = 0x8a;

    this->emit_inst_ri( {op + dst.encoding}, dst, imm);
}


template<typename T>
void Assembler::mov(MemOp dst, T imm) {
    uint8_t op = 0xc7;
    if (sizeof(T) == 1)
        op = 0xc6;
    this->emit_inst_mi( {op}, dst, imm);
}

void Assembler::add(Register dst, Register src) {
    this->emit_inst_rr({0x03}, dst, src);
}

void Assembler::add(Register dst, MemOp src) {
    this->emit_inst_rm({0x03}, dst, src);
}

void Assembler::add(MemOp dst, Register src) {
    this->emit_inst_rm({0x01}, src, dst);
}

template<typename T>
void Assembler::add(Register dst, T imm) {
    uint8_t op = 0x81;
    if (sizeof(T) == 1)
        op = 0x83;
    this->emit_inst_ri({op}, dst, imm);
}

template<typename T>
void Assembler::add(MemOp dst, T imm) {
    uint8_t op = 0x81;
    if (sizeof(T) == 1)
        op = 0x83;
    this->emit_inst_mi({op}, dst, imm);
}

void Assembler::sub(Register dst, Register src) {
    this->emit_inst_rr({0x2b}, dst, src);
}

void Assembler::sub(Register dst, MemOp src) {
    this->emit_inst_rm({0x29}, dst, src);
}

void Assembler::sub(MemOp dst, Register src) {
    this->emit_inst_rm({0x2b}, src, dst);
}

template<typename T>
void Assembler::sub(Register dst, T imm) {
    uint8_t op = 0x81;
    if (dst.size > 1 && sizeof(T) == 1)
        op = 0x83;
    else
        op = 0x80;
    this->emit_inst_ri({op}, 5, dst, imm);
}

template<typename T>
void Assembler::sub(MemOp dst, T imm) {

    uint8_t op = 0x81;
    if (dst.reg.size > 1 && sizeof(T) == 1)
        op = 0x83;
    else
        op = 0x80;
    this->emit_inst_mi({op}, 5, dst, imm);
}

void Assembler::imul(Register dst, Register src) {
    this->emit_inst_rr({0x0f, 0xaf}, dst, src);
}

void Assembler::imul(Register dst, MemOp src) {
    this->emit_inst_rm({0x0f, 0xaf}, dst, src);

}

void Assembler::idiv(Register dst) {
    if (dst.size == 1)
        this->emit_byte(0xf6);
    else
        this->emit_byte(0xf7);
    if (dst.size == 8)
        this->emit_byte( REX(1,0,0,0) );
    this->emit_byte( MOD_BYTE(3, 7, dst.encoding));
}

void Assembler::idiv(MemOp dst) {
    this->idiv(dst.reg);
    /* replace mod byte */
    this->buf[this->buf_size - 1] = MOD_BYTE(dst.disp_size == 4 ? 3 : dst.disp_size, 7, dst.reg.encoding);
}



