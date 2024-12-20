#ifndef ASSEMBLER_X64
#define ASSEMBLER_X64

#include <cstddef>
#include <cstdint>
#include "native_assembler.h"
#include <iostream>
#include <stdexcept>

/* MODR/M
 * mod -> 2 bits
 * reg -> 3 bits
 * rm  -> 3 bits
 * */
#define MOD_BYTE(mod, reg, rm) ( (uint8_t) (mod << 6 | reg << 3 | rm ) )

/* REX prefix */
#define REX(W,R,X,B) ( 0x40 | W << 3 | R << 2 | X << 1 | B )


#define EMIT_REX(dst, rex)  if (dst.size == 8) this->emit_byte(rex); 

namespace X64 {

    enum X64Reg {
        RAX = 0,
        RBX = 3,
        RCX = 1, 
        RDX = 2,
        RSI = 6,
        RDI = 7,
        RBP = 5,
        RSP = 4,
    };


    struct Register {
        uint8_t encoding;
        uint8_t size;

        Register();
        Register(uint8_t encoding, uint8_t size);
    };

    template<typename T = int16_t>
    struct MemOp {
        Register reg;
        T disp;

        MemOp(Register reg, T disp) {
            this->reg = reg;
            this->disp = disp;
        }
        MemOp(Register reg) {
            this->reg = reg;
            this->disp = 0;
        }
    };

    const Register rax(RAX, 8);
    const Register rbx(RBX, 8);
    const Register rcx(RCX, 8);
    const Register rdx(RDX, 8);
    const Register rsi(RSI, 8);
    const Register rdi(RDI, 8);
    const Register rbp(RBP, 8);
    const Register rsp(RSP, 8);

    const Register eax(RAX, 4);
    const Register ebx(RBX, 4);
    const Register ecx(RCX, 4);
    const Register edx(RDX, 4);
    const Register esi(RSI, 4);
    const Register edi(RDI, 4);
    const Register ebp(RBP, 4);
    const Register esp(RSP, 4);


    class Assembler : public NativeAssembler {


        void emit_inst_rr(std::initializer_list<uint8_t> op,  Register dst, Register src);
        template<typename T>
        void emit_inst_ri(std::initializer_list<uint8_t> op , uint8_t opex, Register dst, T imm) {
            if (dst.size > 8 || sizeof(imm) > 8)
                throw std::logic_error("malformed instruction: register/immediate");

            EMIT_REX(dst, REX(1,0,0,0));
            for (auto x : op) {
                this->emit_byte(x);
            }
            this->emit_byte( MOD_BYTE(3, opex, dst.encoding) );
            this->emit_imm(imm);
        }
        template<typename T>
        void emit_inst_rm(std::initializer_list<uint8_t> op, Register dst, MemOp<T> src) {
            if (sizeof(T) > 4 || dst.size > 8)
                throw std::logic_error("malformed instruction: register/memory");
            EMIT_REX(dst, REX(1,0,0,0));
            for (auto x : op) {
                this->emit_byte(x);
            }
            uint8_t mod = src.disp == 0 ? 0 : (sizeof(T) == 4 ? 2 : 1);
            this->emit_byte( MOD_BYTE(mod, dst.encoding, src.reg.encoding));
            if (mod > 0) 
                this->emit_imm(src.disp);

        }

        template<typename T, typename U>
        void emit_inst_mi(std::initializer_list<uint8_t> op, uint8_t opex, MemOp<U> dst, T imm) {
            if (sizeof(imm) > 8 || sizeof(U) > 4 || dst.reg.size > 8) {
                throw std::logic_error("malformed instruction: memory/immediate");
            }

            EMIT_REX(dst.reg, REX(1,0,0,0));
            for (auto x : op) {
                this->emit_byte(x);
            }
            uint8_t mod = dst.disp == 0 ? 0 : (sizeof(U) == 4 ? 2 : 1);
            this->emit_byte( MOD_BYTE(mod, opex, dst.reg.encoding));
            if (mod > 0) 
                this->emit_imm(dst.disp);
            this->emit_imm(imm);
        }
        template<typename T>
        void emit_jcc(T rel, uint8_t op) {
            int rel_size = sizeof(T);
            if (rel_size >= 2) {
                emit_byte(0x0f);
                emit_byte(op + 0x10);
            } else {
                emit_byte(op);
            }
            emit_imm((int32_t) rel);


        }

        public:

            void mov(Register dst, Register src);
            template<typename T>
            void mov(Register dst, MemOp<T> src) {
                this->emit_inst_rm({0x8b}, dst, src);
            }
            template<typename T>
            void mov(MemOp<T> dst, Register src) {
                this->emit_inst_rm({0x89}, src, dst);
            }
            template<typename T>
            void mov(Register dst, T imm) {
                uint8_t op = sizeof(T) == 1 ? 0x8a : 0xb8;
                this->emit_byte( REX(1,0,0,0) );
                this->emit_byte(dst.encoding + op);
                this->emit_imm(imm);
            }

            template<typename T, typename U>
            void mov(MemOp<U> dst, T imm) {
                uint8_t op = sizeof(T) == 1 ? 0xc6: 0xc7;
                this->emit_inst_mi( {op}, 0, dst, imm);
            }

            void add(Register dst, Register src);
            template<typename T>
            void add(Register dst, MemOp<T> src) {
                this->emit_inst_rm({0x03}, dst, src);
            }
            template<typename T>
            void add(MemOp<T> dst, Register src) {
                this->emit_inst_rm({0x01}, src, dst);
            }

            template<typename T>
            void add(Register dst, T imm) {
                uint8_t op = 0x81;
                if (sizeof(T) == 1)
                    op = 0x83;
                this->emit_inst_ri({op}, 0, dst, imm);
            }

            template<typename T, typename U>
            void add(MemOp<U> dst, T imm) {
                uint8_t op = 0x81;
                if (sizeof(T) == 1)
                    op = 0x83;
                this->emit_inst_mi({op}, 0, dst, imm);
            }

            void sub(Register dst, Register src);
            template<typename T>
            void sub(Register dst, MemOp<T> src) {
                this->emit_inst_rm({0x29}, dst, src);
            }
            template<typename T>
            void sub(MemOp<T> dst, Register src) {
                this->emit_inst_rm({0x2b}, src, dst);
            }

            template<typename T>
            void sub(Register dst, T imm) {
                uint8_t op = 0x81;
                if (dst.size > 1 && sizeof(T) == 1)
                    op = 0x83;
                else
                    op = 0x80;
                this->emit_inst_ri({op}, 5, dst, imm);
            }

            template<typename U, typename T>
            void sub(MemOp<U> dst, T imm) {

                uint8_t op = 0x81;
                if (dst.reg.size > 1 && sizeof(T) == 1)
                    op = 0x83;
                else
                    op = 0x80;
                this->emit_inst_mi({op}, 5, dst, imm);
            }

            void imul(Register dst, Register src);
            template<typename T>
            void imul(Register dst, MemOp<T> src) {
                this->emit_inst_rm({0x0f, 0xaf}, dst, src);
            }
            template<typename T>
            void imul(Register dst, T imm) {
                std::runtime_error("imul: not implemented");
            }
            template<typename T>
            void imul(MemOp<T> dst, T imm) {
                std::runtime_error("imul: not implemented");
            }

            void idiv(Register dst);
            template<typename T>
            void idiv(MemOp<T> dst) {
                this->idiv(dst.reg);
                /* replace mod byte */
                this->buf[this->buf_size - 1] = MOD_BYTE(dst.disp_size == 4 ? 3 : dst.disp_size, 7, dst.reg.encoding);
            }

            void ret();
            void hlt();
            void push(Register dst);
            void pop(Register dst);
            template<typename T>
            void call(T label) {
                this->emit_byte(0xe8);
                this->emit_imm(label);
            }
            void cmp(Register ra, Register rb);

            void jmp(Register r);

            template<typename T>
            void jb(T rel) {
                emit_jcc(rel, 0x7f);
            }

            template<typename T>
            void jbe(T rel) {
                emit_jcc(rel, 0x7d);
            }

            template<typename T>
            void jl(T rel) {
                emit_jcc(rel, 0x7c);
            }

            template<typename T>
            void jle(T rel) {
                emit_jcc(rel, 0x7e);
            }
            template<typename T>
            void je(T rel) {
                emit_jcc(rel, 0x74);
            }

            template<typename T>
            void jne(T rel) {
                emit_jcc(rel, 0x75);
            }

            template<typename T>
            void lea(Register dst, MemOp<T> src) {
                emit_inst_rm({0x8d}, dst, src);
            }


            void cmovg(Register dst, Register src) {
                emit_inst_rr({0x0f, 0x4f}, dst, src);
            }
            
            void cmovl(Register dst, Register src) {
                emit_inst_rr({0x0f, 0x4c}, dst, src);
            }

            void cmove(Register dst, Register src) {
                emit_inst_rr({0x0f, 0x44}, dst, src);
            }

            template<typename T>
            void cmovg(Register dst, MemOp<T> src) {
                emit_inst_rm({0x0f, 0x4f}, dst, src);
            }
            
            template<typename T>
            void cmovl(Register dst, MemOp<T> src) {
                emit_inst_rm({0x0f, 0x4c}, dst, src);
            }

            template<typename T>
            void cmove(Register dst, MemOp<T> src) {
                emit_inst_rm({0x0f, 0x44}, dst, src);
            }
            
        };

    }


#endif



