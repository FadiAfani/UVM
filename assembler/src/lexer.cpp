#include "../include/lexer.hpp"

int Lexer::get_col() { return col; }
int Lexer::get_row() { return row; }
vector<Token>& Lexer::get_tokens() { return tokens; }
void Lexer::push_token(const Token& tok) { tokens.push_back(tok); }


Lexer::Lexer(string _file_path) {


    opcode_map["OP_ADDI"] = 0;
    opcode_map["OP_ADD"] = 1;
    opcode_map["OP_FADD"] = 2;
    opcode_map["OP_SUB"] = 3;
    opcode_map["OP_SUBI"] = 4;
    opcode_map["OP_FSUB"] = 5;
    opcode_map["OP_MULT"] = 6;
    opcode_map["OP_FMULT"] = 7;
    opcode_map["OP_DIV"] = 8;
    opcode_map["OP_FDIV"] = 9;
    opcode_map["OP_MOV"] = 10;
    opcode_map["OP_POP"] = 11;
    opcode_map["OP_RET"] = 12;
    opcode_map["OP_CMP"] = 13;
    opcode_map["OP_FCMP"] = 14;
    opcode_map["OP_JMP"] = 15;
    opcode_map["OP_JE"] = 16;
    opcode_map["OP_JL"] = 17;
    opcode_map["OP_JB"] = 18;


    file_path = _file_path;

}

void Lexer::tokenize_word() {

    Token tok;
    char c;
    string& v = tok.get_value();
    while ((c = file.get()) && isalpha(c)) {
        v += c;
    }
    tok.set_type(TOK_WORD);
            
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
                    cout << (string) "unrecognized symbol: " + c + "\n";
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
    cout << lexer->get_tokens()->size();
}


