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
#include "../include/assembler_x64.h"


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
template<class T>
using RegTransferFunc = void (*)(T&, VM* vm, unsigned int, Reg, bool);

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


void transfer_reg_x64(X64::Assembler& x64_asm, VM* vm, unsigned int cpu_reg, Reg vm_reg, bool to_cpu);

template<typename T>
class JITCompiler {
    VM vm;
    std::unordered_map<int, Trace*> trace_map;
    std::stack<Trace*> active_traces;
    bool is_tracing = true;
    T assembler;
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
        JITCompiler() { };
        T& get_assembler() {
            return this->assembler;
        }
        VM& get_vm() {
            return this->vm;
        };
        bool get_is_tracing() {
            return this->is_tracing;
        }
        unsigned int get_cpu_reg(Reg vm_reg) {
            unsigned int r;
            try {
                r = this->x64_reg.at(vm_reg);
            } catch(std::out_of_range& e) {
                std::cout << "get_cpu_reg: unordered_map.at" << std::endl;

            }

            return r;
        }
        void set_memory_addr(uint32_t addr, uint32_t value);
        void dump_output_into_file(const char* fn) {
            std::ofstream fd;
            fd.open(fn, std::ostream::out);
            const char* buff = reinterpret_cast<const char*>(this->assembler.get_buf());
            fd.write(buff, this->assembler.get_buf_size());
            fd.close();
        }

        void gen_x64(const std::vector<uint32_t>& bytecode) {

            unsigned int rae;
            unsigned int rbe;
            unsigned int rde;
            X64::Assembler& assembler = this->get_assembler();

            for (uint32_t inst : bytecode) {
                uint8_t opcode = GET_OPCODE(inst);
                rae= this->get_cpu_reg(GET_RA(inst));
                rbe = this->get_cpu_reg(GET_RB(inst));
                rde = this->get_cpu_reg(GET_RD(inst));
                X64::Register ra(rae, 8);
                X64::Register rb(rbe, 8);
                X64::Register rd(rde, 8);
                //printf("opcode: %d, ra: %d, rb: %d, rd: %d\n", opcode, GET_RA(inst), GET_RB(inst), GET_RD(inst));


                switch(opcode) {
                    case OP_MOV:
                        assembler.mov(rd, ra);
                        break;
                    case OP_MOVI:
                    {
                        uint32_t imm = GET_IMM19(inst);
                        assembler.mov(rd, imm);
                        break;
                    }

                    case OP_ADDI:
                    {
                        uint32_t imm = GET_IMM14(inst);
                        assembler.add(ra, imm);
                        assembler.mov(rd, ra);
                        break;
                    }
                    case OP_SUBI:
                    {
                        uint32_t imm = GET_IMM19(inst);
                        assembler.sub(ra, imm);
                        assembler.mov(rd, ra);
                        break;
                    }
                    case OP_ADD:
                        assembler.add(ra, rb);
                        assembler.mov(rd, ra);
                        break;
                    case OP_SUB:
                        assembler.sub(ra, rb);
                        assembler.mov(rd, ra);
                        break;
                    case OP_MULT:
                        assembler.imul(ra, rb);
                        assembler.mov(rd, ra);
                        break;
                    case OP_DIV:
                        break;
                    case OP_RET:
                        assembler.ret();
                        break;
                    case OP_HALT:
                        assembler.hlt();
                        break;
                    case OP_PUSH:
                        assembler.push(rd);
                        break;
                    case OP_CALL:
                        assembler.call(rd);
                        break;
                    case OP_CMP:
                        break;

                    case OP_LDR:
                    {
                        /* { ldr rd [ra + disp] }
                         * mov rd, vm->memory
                         * add ra, rd
                         * mov rd, [ra + disp]
                         * */
                        int32_t disp = GET_IMM14(inst);
                        assembler.mov(rd, reinterpret_cast<uint64_t>(assembler.get_buf()));
                        assembler.add(ra, rd);
                        assembler.mov(rd, X64::MemOp<int32_t>(ra, disp)); // use int32_t for now
                        break;

                    }
                    case OP_STR:
                    {
                        /* { str rd [ra + disp] } 
                         * mov rbx, vm->memory
                         * add ra rbx
                         * mov [ra + disp], rd
                         * */
                        int32_t disp = GET_IMM14(inst);
                        X64::Register rbx(RBX, 8);
                        assembler.mov(rbx, reinterpret_cast<uint64_t>(assembler.get_buf()));
                        assembler.add(ra, rbx);
                        assembler.mov(X64::MemOp<int32_t>(ra, disp), rd); // use int32_t for now
                        break;
                    }

                    default:
                        fprintf(stderr, "Not Implemented\n");
                        exit(EXIT_FAILURE);

                }
            }
        }
        void gen_arm(const std::vector<uint32_t>& bytecode);

        /* simply transfers the entire state regardless of changes
         * TODO: this method should only transfer modified state 
         * */

        void transfer_reg_state(bool to_cpu, const RegTransferFunc<T> tfunc) {
            for (auto p : this->x64_reg) {
                tfunc(this->assembler, &this->vm, p.second, p.first, to_cpu);
            }
        }
        void map_trace(uint32_t ip, Trace* trace) {
            std::pair<int, Trace*> p (ip, trace);
            this->trace_map.insert(p);
        }
        Trace* get_trace(uint32_t ip) {
            Trace* t;
            try {
                t = this->trace_map.at(ip);
            } catch(std::out_of_range& e) {
                return nullptr;
            }

            return t;
        }
        bool compile_trace(Trace* trace) {

            if (trace == nullptr) 
                return false;


            trace->push_inst(OP_RET << 24);
            trace->set_func(reinterpret_cast<exec_func>(this->assembler.get_buf() + this->assembler.get_buf_size()));
            this->gen_x64(trace->get_bytecode());
            this->dump_output_into_file("binary_dump");
            
            return true;
        }
        void profile(uint32_t inst) {

            uint32_t prev_ip = this->vm.get_reg_as_ref(RIP).as_u32;
            uint32_t ip;
            Trace* trace = nullptr;
            Trace* top;

            switch(GET_OPCODE(inst)) {
                case OP_JB:
                case OP_JE:
                case OP_JL:
                case OP_JBE:
                case OP_JLE:
                case OP_JNE:
                case OP_JMP:
                    this->vm.interpret(inst);
                    ip = this->vm.get_reg_as_ref(RIP).as_u32;
                    if (ip < prev_ip) {
                        trace = this->get_trace(ip);
                        if (trace != nullptr) {
                            trace->inc_heat();
                            this->is_tracing = false;
                        } else {
                            trace = new Trace();
                            this->map_trace(ip, trace);
                            this->is_tracing = true;
                        }

                        this->push_trace(trace);

                    /* handle cases where RIP advances forward 
                     * examples include:
                     * if-else if-else statements in high level languages 
                     * */
                    } else if (ip == prev_ip + 1) {
                        /* condition satisfied */
                        this->is_tracing = true;
                        
                    } else if ((top = this->peek_top_trace()) != nullptr){
                        /* condition unsatisfied */
                        trace = new Trace();
                        trace->set_path_num(top->get_bytecode().size());
                        top->push_path(trace);
                        this->push_trace(trace);
                        this->is_tracing = true;
                    }

                    break;
            }
        }
        void record_inst(uint32_t inst) {

            Trace* trace = this->peek_top_trace();
            if (trace == nullptr || !this->is_tracing) 
                return;

            switch(GET_OPCODE(inst)) {
                case OP_JB:
                case OP_JE:
                case OP_JL:
                case OP_JBE:
                case OP_JLE:
                case OP_JNE:
                case OP_JMP:
                    break;
                
                default:

                    trace->push_inst(inst);
                    break;
            }
        }
        void run() {

            for (;;) {

                uint32_t inst = this->vm.fetch();
                this->profile(inst);
                this->record_inst(inst);


                if (!this->get_is_tracing()) {
                    Trace* tp = this->pop_trace();
                    if (tp != nullptr && tp->get_func() == nullptr) {

                        /* transfer vm state to cpu */
                        this->transfer_reg_state(true, transfer_reg_x64);
                        this->compile_trace(tp);
                        /* transfer cpu state to vm */
                        this->transfer_reg_state(false, transfer_reg_x64);
                    }


                    if (tp != nullptr) {
                        tp->get_func()();
                    }
                    

                } 

        int interp_res = this->vm.interpret(inst);
        if (!interp_res) return;

        

    }
        }
        void get_mod_regs(uint32_t inst);
        void push_trace(Trace* trace) {
            this->active_traces.push(trace);
        }
        Trace* pop_trace() {
            Trace* t = nullptr;
            if (!this->active_traces.empty()) {
                t = this->active_traces.top();
                this->active_traces.pop();
            } 
            return t;
        }
        Trace* peek_top_trace() {
            Trace* t = nullptr;
            if (!this->active_traces.empty()) {
                t = this->active_traces.top();
            }
            return t;
        }

};


#endif
