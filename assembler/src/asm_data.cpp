#include "../include/asm_data.hpp"

AsmData::AsmData() {

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

    reg_map["R0"] = R0;
    reg_map["R1"] = R1;
    reg_map["R2"] = R2;
    reg_map["R3"] = R3;
    reg_map["R4"] = R4;
    reg_map["R5"] = R5;
    reg_map["R6"] = R6;
    reg_map["R7"] = R7;
    reg_map["R8"] = R8;
    reg_map["R9"] = R9;
    reg_map["R10"] = R10;
    reg_map["R11"] = R11;
    reg_map["R12"] = R12;
    reg_map["R13"] = R13;
    reg_map["R14"] = R14;
    reg_map["R15"] = R15;
    reg_map["RIP"] = RIP;
    reg_map["RSP"] = RSP;
    reg_map["RAX"] = RAX;
    reg_map["RBP"] = RBP;
    reg_map["RFLG"] = RFLG;
}
