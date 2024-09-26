#include "../include/assembler_x64.h"

using namespace X64;

Register::Register() {}

Register::Register(uint8_t encoding, uint8_t size) {
    this->encoding = encoding;
    this->size = size;
}

void Assembler::emit_inst_rr(std::initializer_list<uint8_t> op, Register dst, Register src) {
    if (dst.size > 8 || src.size > 8 || dst.size != src.size)
        throw std::logic_error("malformed instruction: register/register"); 
    EMIT_REX(dst, REX(1,0,0,0) );
    for (auto x : op)
        this->emit_byte(x);
    this->emit_byte( MOD_BYTE(3, dst.encoding, src.encoding) );
}


void Assembler::mov(Register dst, Register src) {
    this->emit_inst_rr({0x89}, dst, src);
}

void Assembler::add(Register dst, Register src) {
    this->emit_inst_rr({0x03}, dst, src);
}


void Assembler::sub(Register dst, Register src) {
    this->emit_inst_rr({0x2b}, dst, src);
}


void Assembler::imul(Register dst, Register src) {
    this->emit_inst_rr({0x0f, 0xaf}, dst, src);
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

void Assembler::ret() {
    this->emit_byte(0xc3);
}

void Assembler::hlt() {
    this->emit_byte(0xf4);
}

void Assembler::push(Register dst) {
    this->emit_byte(0x50 + dst.encoding);
}

void Assembler::cmp(Register ra, Register rb) {
    this->emit_inst_rr({0x3b}, ra, rb);
}

void Assembler::jmp(Register reg) {
    emit_byte(0xff);
    emit_byte( MOD_BYTE(3, 4, reg.encoding) );
}
