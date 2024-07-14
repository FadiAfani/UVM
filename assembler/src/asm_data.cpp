#include "../include/asm_data.hpp"

AsmData::AsmData() {

    opcode_map["add"] = OP_ADD;
    opcode_map["addi"] = OP_ADDI;
    opcode_map["fadd"] = OP_FADD;
    opcode_map["sub"] = OP_FSUB;
    opcode_map["subi"] = OP_SUBI;
    opcode_map["fsub"] = OP_FSUB;
    opcode_map["mult"] = OP_MULT;
    opcode_map["fmult"] = OP_FMULT;
    opcode_map["div"] = OP_DIV;
    opcode_map["fdiv"] = OP_FDIV;
    opcode_map["mov"] = OP_MOV;
    opcode_map["pop"] = OP_POP;
    opcode_map["ret"] = OP_RET;
    opcode_map["cmp"] = OP_CMP;
    opcode_map["fcmp"] = OP_FCMP;
    opcode_map["jmp"] = OP_JMP;
    opcode_map["je"] = OP_JE;
    opcode_map["jl"] = OP_JL;
    opcode_map["jb"] = OP_JB;
    opcode_map["halt"] = OP_HALT;
    opcode_map["jne"] = OP_JNE;
    opcode_map["jle"] = OP_JLE;
    opcode_map["jbe"] = OP_JBE;
    opcode_map["movi"] = OP_MOVI;
    opcode_map["ldr"] = OP_LDR;
    opcode_map["str"] = OP_STR;
    opcode_map["push"] = OP_PUSH;
    opcode_map["call"] = OP_CALL;

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
    reg_map["rfp"] = RFP;
    reg_map["rflg"] = RFLG;
}
