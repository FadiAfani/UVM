#ifndef JIT_H
#define JIT_H

#include <cassert>
#include <memory>
#include <optional>
#include <queue>
#include <stdexcept>
#include <stdint.h>
#include <iostream>
#include <sys/mman.h>
#include <unordered_set>
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
#define TEMP_THRES 1
#define GUARD_SIZE 11

// returns the address of the side_exit used
typedef long (*exec_func)();

enum ArchType {
    X86_64,
};


class VM;
class Trace;

struct ExitData {
    uint32_t inst;
    std::shared_ptr<Trace> trace;
};

struct InstData {
    uint32_t ip;
    uint32_t inst;

};


class Trace {
    exec_func func = nullptr;
    std::vector<uint32_t> bytecode;
    std::unordered_set<Reg> saved_regs;
    std::unordered_map<uint32_t, std::unique_ptr<Trace>> paths;
    std::unordered_map<uint32_t, ExitData> exits;
    int execs = 0;
    int trials = 0;
    long last_exec_state = 0;

    public:
        int get_execs();
        int get_trials();
        exec_func get_func();
        const std::vector<uint32_t>& get_bytecode();
        const std::vector<std::unique_ptr<Trace>>& get_paths();
        std::unordered_map<uint32_t, ExitData>& get_exits();
        std::unordered_set<Reg>& get_saved_regs();
        void set_func(exec_func func);
        void register_exit(uint32_t ip, uint32_t dst, uint32_t inst);
        void push_inst(uint32_t ip);
        Reg pop_reg();
        void inc_heat();
        void visit_exit(long ip);
        float get_freq();
        void set_freq(float freq);
        long exec();
        void set_last_exec_state(long state);
        long get_last_exec_state();

};

struct CompareTrace {
    bool operator()(Trace* ta, Trace* tb) const {
        return ta->get_freq() > tb->get_freq();
    }

};

struct HeaderData {
    std::vector<std::unique_ptr<Trace>> traces;
    std::priority_queue<Trace*, std::vector<Trace*>, CompareTrace> tpq;
};


struct TracerState {
    Trace* curt = nullptr;
    long looph = -1;
    bool recording = false;
    int guard_fail_event;

};

template<class T>
class Tracer {
    std::unordered_map<uint32_t, HeaderData> headers;
    JITCompiler<T>* jit;
    TracerState state;


    public:
        Tracer(JITCompiler<T>* jit) : jit(jit) {}

        void map_trace(uint32_t ip, std::unique_ptr<Trace> trace) {
            if (headers.count(ip) == 0) {
                headers.insert( {ip, HeaderData()} );
            }
            headers.at(ip).traces.push_back(std::move(trace));

        }

        void capture_inst(uint32_t inst) {

            if (state.curt == nullptr) 
                return;
            state.curt->push_inst(inst);
        }

        Trace* get_trace(uint32_t ip) {
            Trace* t = nullptr;
            if (headers.count(ip) > 0) {
                if (!headers.at(ip).tpq.empty()) {
                    t = headers.at(ip).tpq.top();
                    headers.at(ip).tpq.pop();
                }
    
            }
            return t;
        }

        void insert_trace(uint32_t ip, Trace* trace) {
            if (headers.count(ip) > 0) {
                headers.at(ip).tpq.push(trace);
            }
        }

        TracerState& get_state() { return this->state; }

};

template<class T>
class Profiler {
    std::unordered_map<uint32_t, int> loop_headers;
    JITCompiler<T>* jit;

    public:
        Profiler(JITCompiler<T>* jit) {
            this->jit = jit;
        }

        void register_header(uint32_t ip) {
            loop_headers.insert( {ip, 0} );
        }

        bool is_loop_header(uint32_t ip) {
            return loop_headers.count(ip) > 0;
        }

        void profile(uint32_t ip) {
            if (is_loop_header(ip)) {
                loop_headers.at(ip)++;
            }
        }

        bool is_hot(uint32_t ip) {
            int temp;
            if (is_loop_header(ip)) {
                temp = loop_headers.at(ip);
            } else {
                return false;
            }
            return temp >= TEMP_THRES;
        }

};

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
    T assembler;
    Profiler<T> profiler;
    Tracer<T> tracer;
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
        JITCompiler() : profiler(this), tracer(this) { };
        T& get_assembler() {
            return this->assembler;
        }
        VM& get_vm() {
            return this->vm;
        };
        Tracer<T>& get_tracer() { return this->tracer; }
        Profiler<T>& get_profiler() { return this->profiler; }
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

        void inline read_vmem_to_x64(const X64::Register reg, Reg vreg) {
            assembler.mov(reg, vm.get_reg_as_ref(vreg));
            assembler.mov(reg, X64::MemOp<>(reg, 0));
        }

        void gen_x64(uint32_t inst) {

            Reg rav = GET_RA(inst);
            Reg rbv = GET_RB(inst);
            Reg rdv = GET_RD(inst);
            unsigned int rae = this->get_cpu_reg(rav);
            unsigned int rbe = this->get_cpu_reg(rbv);
            unsigned int rde = this->get_cpu_reg(rdv);
            X64::Assembler& assembler = this->get_assembler();

            uint8_t opcode = GET_OPCODE(inst);

            switch(opcode) {
                case OP_MOV:
                    read_vmem_to_x64(X64::rax, rav);
                    assembler.mov(X64::rbx, vm.get_reg_as_ref(rdv));
                    assembler.mov(X64::MemOp<>(X64::rbx, 0), X64::rax);
                    break;
                case OP_MOVI:
                {
                    uint32_t imm = GET_IMM19(inst);
                    assembler.mov(X64::rax, vm.get_reg_as_ref(rdv));
                    assembler.mov(X64::MemOp<>(X64::rax, 0), imm);
                    break;
                }

                case OP_ADDI:
                {
                    uint32_t imm = GET_IMM14(inst);
                    read_vmem_to_x64(X64::rax, rav);
                    assembler.add(X64::rax, imm);
                    assembler.mov(X64::rbx, vm.get_reg_as_ref(rdv));
                    assembler.mov(X64::MemOp<uint32_t>(X64::rbx, 0), X64::rax);
                    break;
                }
                case OP_SUBI:
                {
                    uint32_t imm = GET_IMM14(inst);
                    read_vmem_to_x64(X64::rax, rav);
                    assembler.sub(X64::rax, imm);
                    assembler.mov(X64::rbx, vm.get_reg_as_ref(rdv));
                    assembler.mov(X64::MemOp<uint32_t>(X64::rbx, 0), X64::rax);
                    break;
                }
                case OP_ADD:

                    read_vmem_to_x64(X64::rax, rav);
                    read_vmem_to_x64(X64::rbx, rbv);
                    assembler.add(X64::rax, X64::rbx);
                    assembler.mov(X64::rcx, vm.get_reg_as_ref(rdv));
                    assembler.mov(X64::MemOp<uint32_t>(X64::rcx, 0), X64::rax);
                    break;
                case OP_SUB:

                    read_vmem_to_x64(X64::rax, rav);
                    read_vmem_to_x64(X64::rbx, rbv);
                    assembler.sub(X64::rax, X64::rbx);
                    assembler.mov(X64::rcx, vm.get_reg_as_ref(rdv));
                    assembler.mov(X64::MemOp<uint32_t>(X64::rcx, 0), X64::rax);
                    break;
                case OP_MULT:
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
                    read_vmem_to_x64(X64::rax, rav);
                    assembler.push(X64::rax);
                    break;
                case OP_CALL:
                    read_vmem_to_x64(X64::rax, rav);
                    assembler.call(X64::rax);
                    break;
                case OP_CMP:
                    /* this is too much for a single instruction
                     * TODO: find a better solution */

                    read_vmem_to_x64(X64::rax, rav);
                    read_vmem_to_x64(X64::rbx, rbv);
                    assembler.cmp(X64::rbx, X64::rax);
                    assembler.mov(X64::rcx, vm.get_reg_as_ref(RFLG));
                    assembler.mov(X64::rax, (int64_t) 1);
                    assembler.cmovg(X64::rbx, X64::rax);
                    assembler.mov(X64::rax, (int64_t) 0);
                    assembler.cmove(X64::rbx, X64::rax);
                    assembler.mov(X64::rax, (int64_t) -1);
                    assembler.cmovl(X64::rbx, X64::rax);
                    assembler.mov(X64::MemOp<int32_t>(X64::rcx, 0), X64::rbx);
                    break;

                case OP_LDR:
                {
                    /* { ldr rd [ra + disp] }
                     * mov rd, vm->memory
                     * add ra, rd
                     * mov rd, [ra + disp]
                     * */
                    int32_t disp = GET_IMM14(inst);
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
                    break;
                }

                default:
                    fprintf(stderr, "invalid opcode: %d\n", opcode);
                    break;
            
            }
        }
        void gen_arm(const std::vector<uint32_t>& bytecode);

        void transfer_reg_state(Trace* trace, bool to_cpu, const RegTransferFunc<T> tfunc) {
            for (auto p : trace->get_saved_regs()) {
                tfunc(assembler, &vm, x64_reg.at(p), p, to_cpu);
            }
        }
        void compile_trace(Trace* trace) {

            trace->set_func(reinterpret_cast<exec_func>(this->get_assembler().get_buf() + this->get_assembler().get_buf_size()));

            uint32_t ip = vm.get_reg(RIP).as_u32;

            for (auto inst : trace->get_bytecode()) {
                uint8_t opcode = vm.decode(inst);
                if (opcode == OP_JMP)
                    continue;
                if (opcode == OP_JNE 
                    || opcode == OP_JB
                    || opcode == OP_JL
                    || opcode == OP_JE
                    || opcode == OP_JBE
                    || opcode == OP_JLE) {
                    emit_guard(inst);
                }
                else {
                    gen_x64(inst);
                }

            }

            assembler.mov(X64::rax , vm.get_reg_as_ref(RIP));
            assembler.mov(X64::MemOp<>(X64::rax, 0), (int32_t) trace->get_bytecode().size());
            assembler.mov(X64::rax, (int64_t) 0);
            assembler.ret();

        }

        void emit_guard(uint32_t inst) {
            // exit trace and hand control back to the interpreter
            uint8_t opcode = vm.decode(inst);
            switch(opcode) {
                case OP_JB:
                    assembler.jb(GUARD_SIZE);
                    break;
                case OP_JE:
                    assembler.je(GUARD_SIZE);
                    break;
                case OP_JL:
                    assembler.jl(GUARD_SIZE);
                    break;
                case OP_JBE:
                    assembler.jbe(GUARD_SIZE);
                    break;
                case OP_JLE:
                    assembler.jle(GUARD_SIZE);
                    break;
                case OP_JNE:
                    assembler.jne(GUARD_SIZE);
                    break;
                default:
                    fprintf(stderr, "corrupted exit info\n");
                    exit(EXIT_FAILURE);
                    break;
            }
            assembler.mov(X64::rax, (int64_t) -1); // problem with move immediate
            assembler.ret();
        }

        void run() {
            /* Tracer chooses the most frequently executed trace 
             * If it fails it selects the second most frequently executed
             * It goes on in this fashion until traces are exuasted
             * finally it initializes a new trace
             * */

            uint32_t ip, prev_ip = vm.get_reg(RIP).as_u32;
	        Trace* trace;
            TracerState& state = tracer.get_state();

            for (;;) {

                uint32_t inst = this->vm.fetch();
                ip = vm.get_reg(RIP).as_u32;
                if (ip < prev_ip) 
                    profiler.register_header(ip);
        
                profiler.profile(ip);
                trace = tracer.get_trace(ip);

                if ((trace == nullptr || trace->get_last_exec_state() == -1) && profiler.is_hot(ip)) {
                    auto t = std::make_unique<Trace>();
                    state.recording = true;
                    state.curt = t.get();
                    state.looph = ip;
                    tracer.map_trace(ip, std::move(t));
                    tracer.insert_trace(ip, state.curt);
                } else if (state.recording && state.looph == ip) {
                    state.recording = false;
                    state.looph = -1;
                    if (state.curt->get_func() == nullptr) 
                        compile_trace(state.curt);
                    long s = state.curt->exec();
                    state.curt->set_last_exec_state(s);
                    tracer.insert_trace(ip, state.curt);
                    state.curt = nullptr;

                } else if (trace != nullptr) {
                    long s = trace->exec();
                    trace->set_last_exec_state(s);
                    tracer.insert_trace(ip, trace);
                }


                if (state.recording)
                    tracer.capture_inst(inst);

                prev_ip = ip;
                int interp_res = this->vm.interpret(inst);
                if (!interp_res) return;


        }
    }

};


#endif
