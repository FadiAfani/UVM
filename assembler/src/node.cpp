#include "../include/node.hpp"

BinaryExpr::BinaryExpr(Node& _left, Node& _right, Token& _op) : left(_left), right(_right), op(_op) {}

Node& BinaryExpr::get_left() {
    return left;
}


Node& BinaryExpr::get_right() {
    return right;
}


Token& BinaryExpr::get_op() {
    return op;
}

UnaryExpr::UnaryExpr(Node& _expr, Token& _op) : expr(_expr), op(_op) {}

Node& UnaryExpr::get_expr() {
    return expr;
}

Token& UnaryExpr::get_op() {
    return op;
}

Token& Instruction::get_opcode() {
    return opcode;
}

void Instruction::set_rd(const Token* _rd) {
    rd = _rd;
}

void Instruction::set_rs(const Token* _rs) {
    rd = _rs;
}


void Instruction::set_rt(const Token* _rt) {
    rd = _rt;
}

void Instruction::set_imm(const Token* _imm) {
    imm = _imm;
}

Label::Label() {}

void Label::append_inst(const Instruction& inst) {
    insts.push_back(inst);
}

