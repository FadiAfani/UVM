#ifndef JIT_H
#define JIT_H

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

typedef enum arch_type {
    X86_64,
}ArchType;


typedef struct trace {
    uint32_t saddr;
    int heat;
    exec_func func;
    std::vector<uint32_t> bytecode;
    std::stack<Reg> mod_regs;
}Trace;

class VM;

typedef void (*RegTransferFunc)(VM* vm, unsigned int, Reg, bool);

class JITCompiler {
    std::unordered_map<int, Trace*> trace_map;
    uint8_t* mmem;
    size_t mmem_size;
    size_t mmem_cap;
    ArchType target_arch;
    Trace* tp = nullptr;
    bool is_tracing = true;
    bool native_exec;
    const std::unordered_map<Reg, unsigned int> x64_reg;
    const std::unordered_map<Reg, unsigned int> arm_reg;

    public:
        JITCompiler();
        JITCompiler(ArchType arch);
        ArchType get_target_arch();
        bool get_native_exec();
        bool get_is_tracing();
        Trace* get_tp();
        unsigned int get_cpu_reg(Reg vm_reg);
        void set_target_arch(ArchType arch);
        void set_memory_addr(uint32_t addr, uint32_t value);
        void dump_output_into_file(const char* fn); 
        void gen_x64(VM* vm, const std::vector<uint32_t>& bytecode);
        void gen_arm(VM* vm, const std::vector<uint32_t>& bytecode);
        void transfer_reg_x64(VM* vm, unsigned int cpu_reg, Reg vm_reg, bool to_cpu);
        void transfer_reg_state(VM* vm, bool to_cpu, const RegTransferFunc tfunc);
        void map_trace(uint32_t ip, Trace* trace);
        Trace* get_trace(uint32_t ip);
        void compile_trace(VM* vm, Trace* trace);
        void append_code(uint8_t* buffer, size_t len);
        void init_mmem();
        void profile(VM* vm, uint32_t inst);
        void record_inst(VM*, uint32_t inst);
        void get_mod_regs(uint32_t inst);
};

#endif
