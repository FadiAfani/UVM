#include "../include/parser.hpp"
#include "../../include/vm.h"
#include <stdexcept>
#include <iostream>
#include <unordered_map>

Parser::Parser(vector<Token>& _tokens) : tokens(_tokens), asm_data(AsmData()) {
    this->cursor = 0;
}

Token& Parser::read_token() {
    return tokens.at(cursor);
}

Token& Parser::peek_next() {
    return tokens.at(cursor + 1);
}


bool Parser::consume(TokenType type) {
    if (read_token().get_type() == type) {
        cursor++;
        return true;
    }
    return false;
    
}

vector<Instruction>* Parser::get_insts(Token& label) {
    vector<Instruction>* vec = nullptr;
    try {
        vec = &this->labels.at(label.get_value());
    } catch(out_of_range e) {
        cout << e.what() << endl;
    }

    return vec;

}

unordered_map<string, vector<Instruction>>& Parser::get_labels() { return labels;}

Instruction Parser::parse_inst() {
    Token& tok = this->read_token();
    int opcode = this->asm_data.opcode_map.at(tok.get_value());
    consume(TOK_MNEMONIC); // report error
    Instruction inst;
    inst.set_opcode(tok);
    bool err = false;
    switch(opcode) {
        case OP_ADD:
        case OP_SUB:
        case OP_MULT:
        case OP_DIV:
        case OP_FADD:
        case OP_FSUB:
        case OP_FMULT:
        case OP_FDIV:
    
            for (int i = 0; i < 3; i++) {
                inst.set_operand(i, this->read_token());
                err = consume(TOK_REG);
            }
            if (err) {
                // report error
            }
            break;
        
        case OP_ADDI:
        case OP_SUBI:
            inst.set_operand(0, this->read_token());
            err = consume(TOK_REG);

            inst.set_operand(1, this->read_token());
            err = consume(TOK_REG);

            inst.set_operand(2, this->read_token());
            err = consume(TOK_INT) || consume(TOK_FLOAT);

            if (err) {
                // report error
            }

            break;

        
        case OP_JMP:
        case OP_JB:
        case OP_JE:
        case OP_JL:
        case OP_CALL:
            inst.set_operand(0, this->read_token());

            if (!consume(TOK_LABEL)) {
                // report error
            }

            break;

        case OP_CMP:
        case OP_FCMP:
        case OP_MOV:
            inst.set_operand(0, this->read_token());
            err = consume(TOK_REG);

            inst.set_operand(1, this->read_token());
            err = consume(TOK_REG);

            break;

        case OP_POP:
        case OP_RET:
            break;
    }

    return inst;

}

bool Parser::at_end() { return this->cursor >= this->tokens.size() - 1; }

void Parser::parse_label() {
    Token& label = read_token();
    consume(TOK_LABEL);
    consume(TOK_COLON);
    bool indent = false;
    std::vector<Instruction> vec;
    TokenType t;

    while(!this->at_end() && (t = read_token().get_type()) != TOK_LABEL) {
        Instruction inst = parse_inst();
        vec.push_back(inst);
    }
    this->labels[label.get_value()] = vec;

}

void Parser::parse() {
    while(!this->at_end() && read_token().get_type() == TOK_LABEL) {
        parse_label();
    }
}
