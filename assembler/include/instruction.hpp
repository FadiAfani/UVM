#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "../include/token.hpp"
#include <memory>
#include <vector>

#define MAX_OPERANDS 5

class Instruction {
    Token opcode;
    Token operands[MAX_OPERANDS];

    public:
        Instruction();
        Instruction(Token opcode);
        void set_opcode(Token opcode);
        void set_operand(int i, Token operand);
        Token& get_opcode();
        Token* get_operand(int i);

};


#endif
