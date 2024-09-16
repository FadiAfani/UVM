#ifndef ASSEMBLER_X64
#define ASSEMBLER_X64

#include <cstddef>
#include <cstdint>
#include "native_assembler.h"
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

    template<typename T = int8_t>
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


    class Assembler : public NativeAssembler {

        void emit_inst_rr(std::initializer_list<uint8_t> op,  Register dst, Register src);
        template<typename T>
        void emit_inst_ri(std::initializer_list<uint8_t> op , uint8_t opex, Register dst, T imm) {
            if (dst.size > 8 || sizeof(imm) > 8 || dst.size != sizeof(imm))
                throw std::logic_error("malformed instruction: register/immediate");

            EMIT_REX(dst, REX(1,0,0,0));
            for (auto x : op)
                this->emit_byte(x);
            this->emit_byte( MOD_BYTE(3, opex, dst.encoding) );
            this->emit_imm(imm);
        }
        template<typename T>
        void emit_inst_rm(std::initializer_list<uint8_t> op, Register dst, MemOp<T> src) {
            if (sizeof(T) > 4 || dst.size > 8 || dst.size != src.reg.size)
                throw std::logic_error("malformed instruction: register/memory");
            for (auto x : op)
                this->emit_byte(x);

            uint8_t mod = src.disp == 0 ? (sizeof(T) == 4 ? 3 : sizeof(T)) : 0;
            this->emit_byte( MOD_BYTE(mod, dst.encoding, src.reg.encoding));
            if (mod > 0) 
                this->emit_imm(src.disp);
        }

        template<typename T, typename U>
        void emit_inst_mi(std::initializer_list<uint8_t> op, uint8_t opex, MemOp<U> dst, T imm) {
            if (sizeof(imm) > 8 || sizeof(U) > 4 || dst.reg.size > 8)
                throw std::logic_error("malformed instruction: memory/immediate");

            EMIT_REX(dst.reg, REX(1,0,0,0));
            for (auto x : op)
                this->emit_byte(x);
            uint8_t mod = dst.disp == 0 ? (sizeof(U) == 4 ? 3 : sizeof(U)) : 0;
            this->emit_byte( MOD_BYTE(mod, opex, dst.reg.encoding));
            if (mod > 0) 
                this->emit_imm(dst.disp);
            this->emit_imm(imm);
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
                uint8_t op = 0xb8;
                if (sizeof(T) == 1)
                    op = 0x8a;

                this->emit_inst_ri( {op + dst.encoding}, 0, dst, imm);
            }

            template<typename T, typename U>
            void mov(MemOp<U> dst, T imm) {
                uint8_t op = 0xc7;
                if (sizeof(T) == 1)
                    op = 0xc6;
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
            template<typename T>
            void call(T label) {
                this->emit_byte(0xe8);
                this->emit_imm(label);
            }

            
    };

    }


#endif



