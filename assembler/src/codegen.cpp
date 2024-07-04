#include "../include/codegen.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>


Compiler::Compiler(const std::unordered_map<string, std::vector<Instruction>>& labels, string fp) : labels(labels), asm_data(AsmData()) {
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
        case OP_JL:
        case OP_CALL:
        {
            uint32_t imm = this->compile_imm(*inst.get_operand(1));
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
        case OP_POP:
        case OP_RET:
            break;
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
    const std::vector<Instruction>& insts = this->labels.at(label);
    std::vector<uint32_t> vec;
    for (Instruction inst : insts) {
        uint32_t bytes = this->compile_inst(inst);
        vec.push_back(bytes);
    }
    return vec;
}
void Compiler::compile() {
    size_t addr_beg = 0;
    this->output_file.open(this->fp);
    for (const auto& kv : this->labels) {
        this->set_addr(kv.first, addr_beg);
        std::vector<uint32_t> code = this->compile_label(kv.first);
        addr_beg += code.size();
        for (const uint32_t b : code) {
            this->output_file.write((char*) &b, 4);
        }
    }
    uint32_t halt = OP_HALT << 24;
    this->output_file.write((char*) &halt, 4);
    this->output_file.close();
}

void Compiler::set_addr(const string &label, uint32_t addr) {
    this->label_addr[label] = addr;
}

