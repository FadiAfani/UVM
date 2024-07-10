#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "../include/token.hpp"
#include "../include/asm_data.hpp"
#include "../include/error.hpp"
#include "../include/instruction.hpp"
#include <memory>


using namespace std;


class Parser {
    string file_name;
    vector<Token> &tokens;
    size_t cursor;
    AsmData asm_data;
    vector<Error> errors;
    unordered_map<string, pair<uint, vector<Instruction>>> labels;

    public:
        Parser(vector<Token>& _tokens);
        Token& read_token();
        Token& peek_next();
        vector<Error>& get_errors();
        unordered_map<string, pair<uint, vector<Instruction>>>& get_labels();
        bool consume(TokenType type);
        void report_error(string err_msg);
        vector<Instruction>* get_insts(Token& label);
        bool at_end();
        Instruction parse_inst();
        pair<uint, vector<Instruction>> parse_label(uint addr);
        void parse();


};

#endif 
