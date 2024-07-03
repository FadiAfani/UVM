#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "asm_data.hpp"
#include "instruction.hpp"
#include <unordered_map>
#include <vector>
#include <fstream>

class Compiler {
    const std::unordered_map<string, std::vector<Instruction>>& labels;
    std::unordered_map<string, std::vector<uint32_t>> code;
    std::unordered_map<string, uint32_t> label_addr;
    AsmData asm_data;
    ofstream output_file;
    string fp;

    public:
        Compiler(const std::unordered_map<string, std::vector<Instruction>>& labels, string fp);

        uint32_t compile_inst(Instruction inst);
        uint32_t compile_imm(Token& t);
        std::vector<uint32_t> compile_label(const string& label);
        uint32_t get_addr(const string& label);
        void set_addr(const string& label, uint32_t addr);
        void compile();




};

#endif
