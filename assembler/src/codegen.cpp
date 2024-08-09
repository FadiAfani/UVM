#include "../include/codegen.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>


Compiler::Compiler(const std::unordered_map<string, std::pair<uint, std::vector<Instruction>>>& labels, string fp) : labels(labels), asm_data(AsmData()) {
    this->fp = fp;
}

uint32_t Compiler::compile_inst(Instruction inst) {
    Token& t = inst.get_opcode();
    int opcode = this->asm_data.opcode_map.at(t.get_value());
    uint32_t cinst;
    
    switch(opcode) {
        case OP_ADD:
        case OP_SUB:
        case OP_MULT:
        case OP_DIV:
        case OP_FADD:
        case OP_FSUB:
        case OP_FMULT:
        case OP_FDIV:
        {
            uint8_t rd = this->asm_data.reg_map.at(inst.get_operand(0)->get_value());
            uint8_t rs = this->asm_data.reg_map.at(inst.get_operand(1)->get_value());
            uint8_t rt = this->asm_data.reg_map.at(inst.get_operand(2)->get_value());
            cinst = opcode << 24 | rd << 19 | rs << 14 | rt << 9;
            break;
        }
        case OP_ADDI:
        case OP_SUBI:
        {


            uint8_t rd = this->asm_data.reg_map.at(inst.get_operand(0)->get_value());
            uint8_t rs = this->asm_data.reg_map.at(inst.get_operand(1)->get_value());
            uint32_t imm = this->compile_imm(*inst.get_operand(2));
            cinst = opcode << 24 | rd << 19 | rs << 14 | imm;
            break;
        }
        case OP_JMP:
        case OP_JB:
        case OP_JE:
        case OP_JNE:
        case OP_JLE:
        case OP_JBE:
        case OP_JL:
        {
            uint label = this->labels.at(inst.get_operand(0)->get_value()).first;
            cinst = opcode << 24 | label;
            break;

        }
        case OP_CALL:
        {
            uint32_t imm = this->labels.at(inst.get_operand(0)->get_value()).first;
            cinst = opcode << 24 | imm;
            break;
        }
        case OP_CMP:
        case OP_FCMP:
        case OP_MOV:
        {
            uint8_t rd = this->asm_data.reg_map.at(inst.get_operand(0)->get_value());
            uint8_t rs = this->asm_data.reg_map.at(inst.get_operand(1)->get_value());
            cinst = opcode << 24 | rd << 19 | rs << 14;
            break;
        }

        case OP_MOVI:
        {
            uint8_t rd = this->asm_data.reg_map.at(inst.get_operand(0)->get_value());
            uint16_t imm = this->compile_imm(*inst.get_operand(1));
            cinst = opcode << 24 | rd << 19 | imm;
            break;
        }
        case OP_STR:
        case OP_LDR:
        {
            uint8_t rd = this->asm_data.reg_map.at(inst.get_operand(0)->get_value());
            uint8_t rs = this->asm_data.reg_map.at(inst.get_operand(1)->get_value());
            uint16_t imm = 0;
            TokenType sign = inst.get_operand(2)->get_type();
            if (sign == TOK_MINUS) {
                imm = (1 << 13) | compile_imm(*inst.get_operand(3));
            } else {
                imm = compile_imm(*inst.get_operand(3));
            }
            cinst = opcode << 24 | rd << 19 | rs << 14 | imm;
            break;

        }

        case OP_PUSH:
        case OP_POP:
        {
            uint8_t rd = this->asm_data.reg_map.at(inst.get_operand(0)->get_value());
            cinst = opcode << 24 | rd << 19;
            break;
        }
        case OP_RET:
        case OP_HALT:
            cinst = opcode << 24;
            break;

        default:
            cout << "compiler: unrecognized opcode" << endl;
            exit(EXIT_FAILURE);
    }

    return cinst;

}

uint32_t Compiler::compile_imm(Token& t) {
    uint32_t imm;
    if (t.get_type() == TOK_FLOAT) {
        float f = std::stof(t.get_value().data());
        std::memcpy(&imm, &f, 4);
    } else {
        int i = std::stoi(t.get_value().data());
        std::memcpy(&imm, &i, 4);
    }

    return imm;
}

std::vector<uint32_t> Compiler::compile_label(const string& label) { 
    const std::vector<Instruction>& insts = this->labels.at(label).second;
    std::vector<uint32_t> vec;
    for (Instruction inst : insts) {
        uint32_t bytes = this->compile_inst(inst);
        vec.push_back(bytes);
    }
    return vec;
}
void Compiler::compile() {
    this->output_file.open(this->fp);
    std::vector<pair<string, uint>> sorted_keys;

    for (const auto& kv : this->labels) {
        sorted_keys.push_back(pair<string, uint>(kv.first, kv.second.first));
    }
    std::sort(sorted_keys.begin(), sorted_keys.end(), [](auto a, auto b) {return a.second < b.second;});

    /* metadata */
    this->output_file.write((char*) this->code.size(), 4);
    this->output_file.write((char*) this->labels.size(), 4);

    for (const auto& k : sorted_keys) {
        std::vector<uint32_t> code = this->compile_label(k.first);
        this->output_file.write((char*) code.data(), code.size() * 4);
    }

    uint32_t halt = OP_HALT << 24;
    this->output_file.write((char*) &halt, 4);
    this->output_file.close();
}


