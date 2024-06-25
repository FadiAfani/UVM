#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "../include/token.hpp"
#include "../include/node.hpp"
#include "../include/asm_data.hpp"
#include <memory>


using namespace std;


class Parser {
    vector<Token> &tokens;
    size_t cursor;
    AsmData asm_data;

    public:
        Parser(vector<Token>& _tokens);
        Token& read_token();
        Token& peek_next();
        bool consume(TokenType type);

        unique_ptr<Instruction> parse_inst();


};

#endif 
