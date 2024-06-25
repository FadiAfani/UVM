#ifndef TOKEN_H
#define TOKEN_H

#include <string>

using namespace std;

typedef enum TokenType {
    TOK_REG,
    TOK_MNEMONIC,
    TOK_LABEL,
    TOK_INT,
    TOK_FLOAT,
    TOK_MINUS,
    TOK_PLUS,
    TOK_MULT,
    TOK_DIV,
    TOK_LBRAC,
    TOK_RBRAC,
}TokenType;

class Token {
    string value;
    int line = 0;
    int tok_beg = 0;
    TokenType type;

    public:
        Token(string _value, TokenType _type);
        Token();
        string& get_value();
        int get_line(); 
        int get_tok_beg(); 
        TokenType get_type(); 
        
        void set_value(string _value);
        void set_type(TokenType _type); 
};


#endif
