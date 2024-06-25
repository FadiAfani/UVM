#ifndef NODE_H
#define NODE_H

#include "../include/token.hpp"

#include <vector>

using namespace std;

class Node {

};

class BinaryExpr : Node {
    Node& left;
    Node& right;
    Token& op;

    BinaryExpr(Node& _left, Node& _right, Token& _op);

    public:
        Node& get_left();
        Node& get_right();
        Token& get_op();

};

class UnaryExpr {
    Node& expr;
    Token& op;

    UnaryExpr(Node& _expr, Token& _op);

    public:
        Node& get_expr();
        Token& get_op();

};

class Instruction : Node {
    Token& opcode;
    Token* rd = nullptr;
    Token* rs = nullptr;
    Token* rt = nullptr;
    Token* imm = nullptr;
    
    
    public:
        Token& get_opcode();
        void set_rd(const Token* _rd);
        void set_rs(const Token* _rs);
        void set_rt(const Token* _rt);
        void set_imm(const Token* _imm);

    Instruction(Token& _opcode);


};


class Label : Node {
    vector<Instruction> insts;

    Label();

    public:
        void append_inst(const Instruction& inst);
};



#endif
