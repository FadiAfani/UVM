#include "../include/token.hpp"
#include <stdlib.h>
#include <iostream>


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

void Token::set_line(int _line) {
    line = _line;
}

void Token::set_type(TokenType _type) {
    type = _type;
}

void Token::set_tok_beg(int _tok_beg) {
    tok_beg = _tok_beg;
}

void Token::print() {
    cout << "TOKEN: " + to_string(this->get_type()) << endl;
}

