#include "../include/token.hpp"
#include <stdlib.h>


Token::Token(string _value, TokenType _type) {
    value = _value;
    type = _type;
}

Token::Token() {}

string& Token::get_value() {
    return value;
}

int Token::get_line() {
    return line;
}

int Token::get_tok_beg() {
    return tok_beg;
}

TokenType Token::get_type() {
    return type;
}

