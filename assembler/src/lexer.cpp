#include "../include/lexer.hpp"
#include <unordered_map>
#include <iostream>

int Lexer::get_col() { return col; }
int Lexer::get_row() { return row; }
vector<Token>& Lexer::get_tokens() { return tokens; }
void Lexer::push_token(Token tok) { tokens.push_back(tok); }


Lexer::Lexer(string _file_path) : asm_data(AsmData()) {

    file_path = _file_path;
}

void Lexer::tokenize_word(Token& tok) {

    char c;
    string& v = tok.get_value();
    while ((c = file.peek()) && isalpha(c) || isdigit(c) || c == '_') {
        file.get();
        v += c;
    }

    if (asm_data.reg_map.count(v) > 0) {
        tok.set_type(TOK_REG);
    } else if (asm_data.opcode_map.count(v) > 0){
        tok.set_type(TOK_MNEMONIC);
    } else {
        tok.set_type(TOK_LABEL);
    }
            
}


void Lexer::tokenize_number(Token& tok) {
    char c;
    string& v = tok.get_value();
    while ((c = file.peek()) && isdigit(c)) {
        file.get();
        v += c;
    }

    if (c != '.') {
        tok.set_type(TOK_INT);
        return;
    }
    tok.set_type(TOK_FLOAT);
    v += ".";

    while ((c = file.peek()) && isdigit(c)) {
        file.get();
        v += c;
    }
    
}


void Lexer::tokenize() {

    file.open(file_path);
    char c;
    while ((c = file.get()) > -1) {
        Token tok;
        tok.set_line(col);
        tok.set_tok_beg(row);
        switch(c) {
            case '[':
                tok.set_type(TOK_LBRAC);
                push_token(tok);
                break;
            case ']':
                tok.set_type(TOK_RBRAC);
                push_token(tok);
                break;
            case ',':
                tok.set_type(TOK_COMMA);
                push_token(tok);
                break;
            case ':':
                tok.set_type(TOK_COLON);
                push_token(tok);
                break;
            case '.':
                tok.set_type(TOK_DOT);
                push_token(tok);
                break;
            case '+':
                tok.set_type(TOK_PLUS);
                push_token(tok);
                break;

            case '-':
                tok.set_type(TOK_MINUS);
                push_token(tok);
                break;
            default:
                if (isalpha(c)) {
                    file.unget();
                    tokenize_word(tok);
                    push_token(tok);
                    break;
                } else if (isdigit(c)) {
                    file.unget();
                    tokenize_number(tok);
                    push_token(tok);
                    break;
                
                } else if (isspace(c)) {
                    break;
                } else {
                    cout << (string) "unrecognized symbol: " + c + "\n" << endl;
                    exit(EXIT_FAILURE);
                }

        }

    }

    file.close();
}




