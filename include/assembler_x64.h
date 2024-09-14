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
    };

    struct MemOp {
        Register reg;
        union {
            int32_t as_i32;
            int16_t as_i16;
            int8_t as_i8;
        }disp;
        uint8_t disp_size;
    };


    class Assembler : public NativeAssembler {

        void emit_inst_rr(std::initializer_list<uint8_t>,  Register dst, Register src);
        template<typename T>
        void emit_inst_ri(std::initializer_list<uint8_t>, uint8_t opex, Register dst, T imm);
        template<typename T>
        void emit_inst_mi(std::initializer_list<uint8_t>, uint8_t opex, MemOp dst, T imm);
        void emit_inst_rm(std::initializer_list<uint8_t>, Register dst, MemOp src);

        public:
            Assembler();
            void mov(Register dst, Register src);
            void mov(Register dst, MemOp src);
            void mov(MemOp dst, Register src);
            template<typename T>
            void mov(Register dst, T imm);
            template<typename T>
            void mov(MemOp dst, T imm);

            void add(Register dst, Register src);
            void add(Register dst, MemOp src);
            void add(MemOp dst, Register src);
            template<typename T>
            void add(Register dst, T imm);
            template<typename T>
            void add(MemOp dst, T imm);

            void sub(Register dst, Register src);
            void sub(Register dst, MemOp src);
            void sub(MemOp dst, Register src);
            template<typename T>
            void sub(Register dst, T imm);
            template<typename T>
            void sub(MemOp dst, T imm);

            void imul(Register dst, Register src);
            void imul(Register dst, MemOp src);
            template<typename T>
            void imul(Register dst, T imm);
            template<typename T>
            void imul(MemOp dst, T imm);

            void idiv(Register dst);
            void idiv(MemOp dst);

            void ret();
            void hlt();
            void push(Register dst);
            void call(Register dst);

            
    };

    }


#endif



