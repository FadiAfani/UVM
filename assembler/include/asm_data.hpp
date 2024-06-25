#ifndef ASM_DATA_H
#define ASM_DATA_H

#include <string>
#include <unordered_map>
#include "../../include/vm.h"


typedef struct AsmData {
    std::unordered_map<std::string, uint> opcode_map;
    std::unordered_map<std::string, uint> reg_map;

    AsmData();
}AsmData;

#endif
