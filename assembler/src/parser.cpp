#include "../include/parser.hpp"
#include "../../include/vm.h"

Parser::Parser(vector<Token>& _tokens) : tokens(_tokens) {}

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

unique_ptr<Instruction> Parser::parse_inst() {
    unique_ptr<Instruction> inst;
    Token& t = read_token();
    if (consume(TOK_MNEMONIC)) {
        inst = make_unique<Instruction>(t);
        switch(asm_data.reg_map.at(t.get_value())) {
            case OP_ADD:
            case OP_SUB:
            case OP_MULT:
            case OP_DIV:
            case OP_FADD:
            case OP_FDIV:
            case OP_FSUB:
            case OP_FMULT:
                Token* rd = &read_token();
                if (!consume(TOK_REG)) {
                    /* report error */
                }
                inst->set_rd(rd);

                Token* rs = &read_token();
                if (!consume(TOK_REG)) {
                    /* report error */
                }
                inst->set_rd(rs);

                Token* rt = &read_token();
                if (!consume(TOK_REG)) {
                    /* report error */
                }
                inst->set_rd(rt);


                break;

        }

    }
    return nullptr;
}


