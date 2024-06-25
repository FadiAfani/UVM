#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "../include/token.hpp"
#include "../include/node.hpp"
#include <memory>


using namespace std;


class Parser {
    vector<Token> &tokens;
    size_t cursor = 0;

    public:
        Parser(vector<Token>& _tokens);
        Token& read_token();
        Token& peek_next();
        bool consume(Token& t);

        unique_ptr<Instruction> parse_inst();





};

#endif 
