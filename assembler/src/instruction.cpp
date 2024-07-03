#include "../include/instruction.hpp"
#include <stdexcept>
#include <iostream>

Instruction::Instruction() {}

Instruction::Instruction(Token opcode) {
    this->opcode = opcode;
}

void Instruction::set_opcode(Token opcode) {
    this->opcode = opcode;
}

Token& Instruction::get_opcode() { return this->opcode; }

Token* Instruction::get_operand(int i) {

    try {

        if (i < MAX_OPERANDS) { 
            return this->operands + i;
        }

        throw new out_of_range("not a valid operand");

    } catch(out_of_range e) {
        cout << "not a valid operand: index out of bounds" << endl;
    }

    return nullptr;
}

void Instruction::set_operand(int i, Token operand) {
    try {

        if (i < MAX_OPERANDS) {
            this->operands[i] = operand;
        } else {
            throw new out_of_range("not a valid operand");
        }

    } catch(out_of_range e) {
        cout << "not a valid operand: index out of bounds" << endl;
    }
}
