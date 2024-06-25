#include "../include/parser.hpp"

Parser::Parser(vector<Token>& _tokens) : tokens(_tokens) {}

Token& Parser::read_token() {
    return tokens.at(cursor);
}

Token& Parser::peek_next() {
    return tokens.at(cursor + 1);
}


bool Parser::consume(Token& t) {
    if (read_token().get_type() == t.get_type()) {
        cursor++;
        return true;
    }
    return false;
    
}

unique_ptr<Instruction> Parser::parse_inst() {
    return nullptr;
}


