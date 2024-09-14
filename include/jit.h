#ifndef JIT_H
#define JIT_H

#include <memory>
#include <optional>
#include <queue>
#include <stdexcept>
#include <stdint.h>
#include <iostream>
#include <sys/mman.h>
#include "../lib/vector.h"
#include "../include/vm.h"


/* MODR/M
 * mod -> 2 bits
 * reg -> 3 bits
 * rm  -> 3 bits
 * */
#define MOD_BYTE(mod, reg, rm) ( (uint8_t) (mod << 6 | reg << 3 | rm ) )

/* REX prefix */
#define REX(W,R,X,B) ( 0x40 | W << 3 | R << 2 | X << 1 | B )

typedef void (*exec_func)();

enum ArchType {
    X86_64,
};


class Trace {
    int path_num;
    int heat;
    exec_func func = nullptr;
    std::vector<uint32_t> bytecode;
    std::stack<Reg> mod_regs;
    std::vector<Trace*> paths;

    public:
        int get_heat();
        exec_func get_func();
        const std::vector<uint32_t>& get_bytecode();
        void set_path_num(int n);
        void set_func(exec_func func);
        void push_inst(uint32_t inst);
        void push_path(Trace* trace);
        void inc_heat();


};

class VM;

typedef void (*RegTransferFunc)(JITCompiler*, VM* vm, unsigned int, Reg, bool);

enum X64Reg {
    RAX = 0,
    RBX = 3,
    RCX = 1, 
    RDX = 2,
    RSI = 6,
    RDI = 7,
    RBP = 5,
    RSP = 4,
};


class JITCompiler {
    VM vm;
    std::unordered_map<int, Trace*> trace_map;
    std::stack<Trace*> active_traces;
    ArchType target_arch;
    uint8_t* buf = nullptr;
    size_t buf_size = 0;
    size_t buf_cap = 0;
    bool is_tracing = true;
    const std::unordered_map<Reg, unsigned int> x64_reg = {
        {R0, RAX},
        {R1, RBX},
        {R2, RCX},
        {R3, RDX},
        {R4, RSI},
        {R5, RDI},
        {R6, RBP},
        {R7, RSP}

    };

    const std::unordered_map<Reg, unsigned int> arm_reg;

    public:
        JITCompiler();
        JITCompiler(ArchType arch);
        ArchType get_target_arch();
        VM& get_vm();
        bool get_is_tracing();
        unsigned int get_cpu_reg(Reg vm_reg);
        uint8_t* get_buf();
        void set_target_arch(ArchType arch);
        void set_memory_addr(uint32_t addr, uint32_t value);
        void dump_output_into_file(const char* fn); 
        void gen_x64(const std::vector<uint32_t>& bytecode);
        void gen_arm(const std::vector<uint32_t>& bytecode);
        void transfer_reg_state(bool to_cpu, const RegTransferFunc tfunc);
        void map_trace(uint32_t ip, Trace* trace);
        Trace* get_trace(uint32_t ip);
        bool compile_trace(Trace* trace);
        void emit(uint8_t* buffer, size_t len);
        void init_mmem();
        void profile(uint32_t inst);
        void record_inst(uint32_t inst);
        void run();
        void get_mod_regs(uint32_t inst);
        void push_trace(Trace* trace);
        Trace* pop_trace();
        Trace* peek_top_trace();

};

void transfer_reg_x64(JITCompiler* jit, VM* vm, unsigned int cpu_reg, Reg vm_reg, bool to_cpu);

#endif
