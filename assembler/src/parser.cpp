#include "../include/parser.hpp"
#include "../../include/vm.h"
#include <stdexcept>
#include <iostream>
#include <unordered_map>
#include <optional>


Parser::Parser(vector<Token>& _tokens) : tokens(_tokens), asm_data(AsmData()) {
    this->cursor = 0;
}


Token& Parser::read_token() {
    if (this->at_end()) { 
        this->print_errors(); 
        exit(EXIT_FAILURE);
    }
    return tokens.at(cursor);
}

Token& Parser::peek_next() {
    if (this->at_end()) { 
        this->print_errors(); 
        exit(EXIT_FAILURE);
    }
    return tokens.at(cursor + 1);
}

vector<Error>& Parser::get_errors() {
    return this->errors;
}

void Parser::print_errors() {
    for (auto e : this->errors) {
        cout << e.to_string() << endl;
    }
}

void Parser::report_error(string err_msg, int line) {
    Error err = Error(this->file_name, err_msg, line);
    this->errors.push_back(err);
}


inline bool Parser::consume(TokenType type, string err_msg) {
    Token tok = read_token();
    if (tok.get_type() == type) {
        cursor++;
        return true;
    }
    report_error(err_msg, tok.get_line());
    return false;
    
}


inline bool Parser::consume_optional(TokenType type) {
    Token tok = read_token();
    if (tok.get_type() == type) {
        cursor++;
        return true;
    }
    return false;
    
}

inline bool Parser::consume_any(vector<TokenType> types, string err_msg) {
    Token tok = read_token();

    for (auto t : types) {
        if (consume_optional(t)) return true;
    }

    report_error(err_msg, tok.get_line());

    return false;
}

vector<Token> Parser::parse_mem_addr() {
    Token addr, sign, shift;
    consume(TOK_LBRAC, "expected a '[' symbol");
    addr = read_token();
    consume_any({TOK_LABEL, TOK_INT}, "expected an address");
    sign = read_token();
    bool is_shifted = consume_optional(TOK_PLUS) || consume_optional(TOK_MINUS);
    if (is_shifted) {
        shift = read_token();
        consume(TOK_INT, "expected a shift amount after the sign");
    }
    consume(TOK_RBRAC, "expected a ']' symbol");

    return {addr, sign, shift};

}


vector<Instruction>* Parser::get_insts(Token& label) {
    vector<Instruction>* vec = nullptr;
    try {
        vec = &this->labels.at(label.get_value()).second;
    } catch(out_of_range e) {
        cout << e.what() << endl;
    }

    return vec;

}



unordered_map<string, pair<uint, vector<Instruction>>>& Parser::get_labels() { return labels;}

Instruction Parser::parse_inst() {
    Token& tok = read_token();
    int opcode = this->asm_data.opcode_map.at(tok.get_value());
    consume(TOK_MNEMONIC, "instructions must start with an opcode");
    Instruction inst;
    inst.set_opcode(tok);
    switch(opcode) {
        case OP_ADD:
        case OP_SUB:
        case OP_MULT:
        case OP_DIV:
        case OP_FADD:
        case OP_FSUB:
        case OP_FMULT:
        case OP_FDIV:


            inst.set_operand(0, read_token());
            consume(TOK_REG, "missing rd register");

            inst.set_operand(1, read_token());
            consume(TOK_REG, "missing rs register");

            inst.set_operand(2, read_token());
            consume(TOK_REG, "missing rt register");

    
            break;
        
        case OP_ADDI:
        case OP_SUBI:
            inst.set_operand(0, read_token());
            consume(TOK_REG, "missing rd register");

            inst.set_operand(1, read_token());
            consume(TOK_REG, "missing rs register");

            inst.set_operand(2, read_token());
            consume_any({TOK_INT, TOK_FLOAT}, "missing an immediate value");

            break;

        
        case OP_JMP:
        case OP_JB:
        case OP_JE:
        case OP_JL:
        case OP_JNE:
        case OP_JLE:
        case OP_JBE:
        case OP_CALL:
            inst.set_operand(0, read_token());
            consume(TOK_LABEL, "missing an address");

            break;

        case OP_CMP:
        case OP_FCMP:
        case OP_MOV:
            inst.set_operand(0, read_token());
            consume(TOK_REG, "missing rd register");

            inst.set_operand(1, read_token());
            consume(TOK_REG, "missing rs register");

            break;

        case OP_MOVI:
            inst.set_operand(0, read_token());
            consume(TOK_REG, "missing rd register");

            inst.set_operand(1, read_token());
            consume_any({TOK_INT, TOK_FLOAT}, "missing an immediate value");
            break;

        case OP_LDR:
        case OP_STR:
        {
            inst.set_operand(0, read_token());
            consume(TOK_REG, "missing rd register");
            vector<Token> addr = parse_mem_addr();
            for (int i = 0; i < 3; i++) {
                inst.set_operand(i + 1, addr.at(i));
            }
            break;

        }

        case OP_POP:
        case OP_RET:
        case OP_HALT:
            break;
    }

    return inst;

}

bool Parser::at_end() { return this->cursor > this->tokens.size() - 1; }

pair<uint, vector<Instruction>> Parser::parse_label(uint addr) {
    /* not actually an optional
     * we just don't want to type an empty error message
     * */

    consume_optional(TOK_LABEL);
    cout << read_token().get_value() << endl;
    consume(TOK_COLON, "expected a colon after a label");
    bool indent = false;
    std::vector<Instruction> vec;
    TokenType t;

    while(!this->at_end() && (t = read_token().get_type()) != TOK_LABEL) {
        Instruction inst = parse_inst();
        vec.push_back(inst);
    }
    return pair<uint, vector<Instruction>>(addr, vec);

}

void Parser::parse() {
    uint next_addr = 0;
    while(!this->at_end() && read_token().get_type() == TOK_LABEL) {
        Token& label = read_token();
        pair<uint, vector<Instruction>> p = parse_label(next_addr);
        next_addr += p.second.size();
        this->labels[label.get_value()] = p;
    }
}
