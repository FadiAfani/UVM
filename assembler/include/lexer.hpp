#ifndef LEXER_H
#define LEXER_H

#include "../include/token.hpp"
#include "asm_data.hpp"
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
        AsmData asm_data;

    public:
        int get_col();
        int get_row();
        vector<Token>& get_tokens();
        void push_token(Token tok);
        Lexer(string _file_path);
        void tokenize_word(Token& tok);
        void tokenize_number(Token& tok);
        void tokenize();



};


#endif
