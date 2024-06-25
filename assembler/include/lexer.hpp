#ifndef LEXER_H
#define LEXER_H

#include "../include/token.hpp"
#include <fstream>
#include <vector>
#include <unordered_map>
#include <iostream>

using namespace std;


class Lexer {

    private:
        int col = 0;
        int row = 0;
        vector<Token> tokens;
        string file_path;
        ifstream file;
        unordered_map<string, uint8_t> opcode_map;
        unordered_map<string, uint8_t> reg_map;

    public:
        int get_col();
        int get_row();
        vector<Token>& get_tokens();
        void push_token(const Token& tok);
        Lexer(string _file_path);
        void tokenize_word();
        void tokenize_number();
        void tokenize();



};


#endif
