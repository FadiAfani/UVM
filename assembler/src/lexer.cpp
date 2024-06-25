#include "../include/lexer.hpp"

int Lexer::get_col() { return col; }
int Lexer::get_row() { return row; }
vector<Token>& Lexer::get_tokens() { return tokens; }
void Lexer::push_token(const Token& tok) { tokens.push_back(tok); }


Lexer::Lexer(string _file_path) {

    file_path = _file_path;
    row = 0;
    col = 0;

}

void Lexer::tokenize_word() {

    Token tok;
    char c;
    string& v = tok.get_value();
    while ((c = file.get()) && isalpha(c)) {
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


void Lexer::tokenize_number() {
    Token tok; 
    char c;
    string& v = tok.get_value();
    while ((c = file.get()) && isdigit(c)) {
        v += c;
    }
    if (c != '.') {
        tok.set_type(TOK_INT);
        push_token(tok);
        return;
    }
    tok.set_type(TOK_FLOAT);
    v += ".";

    while ((c = file.get()) && isdigit(c)) {
        v += c;
    }

    push_token(tok);
    
}


void Lexer::tokenize() {

    file.open(file_path);
    char c;
    while ((c = file.get()) > -1) {
        switch(c) {
            default:
                if (isalpha(c)) {
                    file.unget();
                    tokenize_word();

                }
                else if (isdigit(c)) {
                    file.unget();
                    tokenize_number();
                
                } else if (isspace(c)) {
                    break;
                } else {
                    cout << (string) "unrecognized symbol: " + c + "\n" << endl;
                    return;
                }

                break;
        }
    }
    file.close();
}


int main() {
    Lexer* lexer = new Lexer("../test.uasm");
    lexer->tokenize();
    cout << lexer->get_tokens().size() << endl;
}


