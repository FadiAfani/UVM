#include "../include/asm_data.hpp"

AsmData::AsmData() {

    opcode_map["add"] = 0;
    opcode_map["addi"] = 1;
    opcode_map["fadd"] = 2;
    opcode_map["sub"] = 3;
    opcode_map["subi"] = 4;
    opcode_map["fsub"] = 5;
    opcode_map["mult"] = 6;
    opcode_map["fmult"] = 7;
    opcode_map["div"] = 8;
    opcode_map["fdiv"] = 9;
    opcode_map["mov"] = 10;
    opcode_map["pop"] = 11;
    opcode_map["ret"] = 12;
    opcode_map["cmp"] = 13;
    opcode_map["fcmp"] = 14;
    opcode_map["jmp"] = 15;
    opcode_map["je"] = 16;
    opcode_map["jl"] = 17;
    opcode_map["jb"] = 18;
    opcode_map["halt"] = 19;

    reg_map["r0"] = R0;
    reg_map["r1"] = R1;
    reg_map["r2"] = R2;
    reg_map["r3"] = R3;
    reg_map["r4"] = R4;
    reg_map["r5"] = R5;
    reg_map["r6"] = R6;
    reg_map["r7"] = R7;
    reg_map["r8"] = R8;
    reg_map["r9"] = R9;
    reg_map["r10"] = R10;
    reg_map["r11"] = R11;
    reg_map["r12"] = R12;
    reg_map["r13"] = R13;
    reg_map["r14"] = R14;
    reg_map["r15"] = R15;
    reg_map["rip"] = RIP;
    reg_map["rsp"] = RSP;
    reg_map["rax"] = RAX;
    reg_map["rbp"] = RBP;
    reg_map["rflg"] = RFLG;
}
