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

typedef void (*exec_func)();

enum ArchType {
    X86_64,
};


class VM;

class Trace {
    uint32_t* guard_inst = nullptr;
    exec_func func = nullptr;
    std::vector<uint32_t> bytecode;
    std::unordered_set<Reg> saved_regs;
    std::vector<std::unique_ptr<Trace>> paths;

    public:
        int get_heat();
        exec_func get_func();
        const std::vector<uint32_t>& get_bytecode();
        const std::vector<std::unique_ptr<Trace>>& get_paths();
        uint32_t* get_guard_inst();
        void set_guard_inst(uint32_t* inst);
        std::unordered_set<Reg>& get_saved_regs();
        void set_func(exec_func func);
        void push_inst(uint32_t inst);
        void push_path(std::unique_ptr<Trace> trace);
        Reg pop_reg();
        void inc_heat();


};

template<class T>
class Tracer {
    std::unordered_map<uint32_t, std::unique_ptr<Trace>> traces;
    JITCompiler<T>* jit;
    std::stack<Trace*> active_traces;
    uint32_t* next_guard = nullptr;
    bool tracing = false;


    public:
        Tracer(JITCompiler<T>* jit) {
            this->jit = jit;
        }
        bool get_tracing() {
            return tracing;
        }
        std::stack<Trace*>& get_active_traces() { return this->active_traces; }
        void set_tracing(bool tracing) {
            this->tracing = tracing;
        }

        void map_trace(uint32_t ip, std::unique_ptr<Trace> trace) {
            traces.insert( {ip, std::move(trace)} );
        }

        void capture_inst(uint32_t inst) {
            Trace* trace = peek_top_trace();

            if (trace == nullptr || !tracing) 
                return;
            preserve_reg(trace, inst);
            uint8_t opcode = jit->get_vm().decode(inst);
            trace->push_inst(inst);

        }

        void push_trace(Trace* trace) {
            active_traces.push(trace);
        }
        Trace* pop_trace() {
            Trace* t = nullptr;
            if (!active_traces.empty()) {
                t = active_traces.top();
                active_traces.pop();
            } 
            return t;
        }
        Trace* peek_top_trace() {
            Trace* t = nullptr;
            if (!active_traces.empty()) {
                t = active_traces.top();
            }
            return t;
        }
        void preserve_reg(Trace* trace, uint32_t inst) {
            if (trace == nullptr) return;
            uint8_t opcode = jit->get_vm().decode(inst);
            Reg rd = GET_RD(inst);
            if (opcode == OP_MOV
                || opcode == OP_MOVI
                || opcode == OP_ADD
                || opcode == OP_ADDI
                || opcode == OP_SUB
                || opcode == OP_SUBI
                || opcode == OP_MULT
                || opcode == OP_DIV
               ) {
                trace->get_saved_regs().insert(rd);
            }
        }

        Trace* get_trace(uint32_t ip) {
            Trace* t = nullptr;
            try {
                t = traces.at(ip).get();
            } catch(std::out_of_range& e) {
                return nullptr;
            }

            return t;
        }

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
            loop_headers.insert( {ip, 1} );
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

        void gen_x64(uint32_t inst) {

            unsigned int rae;
            unsigned int rbe;
            unsigned int rde;
            X64::Assembler& assembler = this->get_assembler();

            uint8_t opcode = GET_OPCODE(inst);
            rae = this->get_cpu_reg(GET_RA(inst));
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
                    uint64_t imm = GET_IMM19(inst);
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
                    assembler.cmp(ra, rb);
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
            if (trace == nullptr)
                return;
            for (auto& p : trace->get_paths()) {
                compile_trace(p.get());
            }
            /* compile side exits first
             * TODO: rewrite iteratively */
            trace->push_inst(OP_RET << 24);
            trace->set_func(reinterpret_cast<exec_func>(assembler.get_buf() + assembler.get_buf_size()));

            transfer_reg_state(trace, true, transfer_reg_x64);
            for (auto inst : trace->get_bytecode()) {
                uint8_t opcode = vm.decode(inst);
                if (opcode == OP_JB
                    || opcode == OP_JBE
                    || opcode == OP_JL
                    || opcode == OP_JLE
                    || opcode == OP_JE
                    || opcode == OP_JNE
                    || opcode == OP_JMP
                    ) {
                    emit_guard(trace, inst);
                } else {
                    gen_x64(inst);
                }

            }
            transfer_reg_state(trace, false, transfer_reg_x64);
        }

        void emit_guard(Trace* trace, uint32_t inst) {
            if (trace == nullptr) return;
            uint8_t opcode = get_vm().decode(inst);
            uint8_t* tptr = reinterpret_cast<uint8_t*>(trace->get_func());
            X64::Register rax(X64::RAX, 8);
            X64::Register rbx(X64::RBX, 8);
            switch(opcode) {
                case OP_JB:
                    assembler.jle((int8_t) 2);
                    break;
                case OP_JE:
                    assembler.jne((int8_t) 2);
                    break;
                case OP_JL:
                    assembler.jbe((int8_t) 2);
                    break;
                case OP_JBE:
                    assembler.jl((int8_t) 2);
                    break;
                case OP_JLE:
                    assembler.jb((int8_t) 2);
                    break;
                case OP_JNE:
                    assembler.je((int8_t) 2);
                    break;
            }
            assembler.mov(rax, tptr);
            assembler.jmp(rax);

        }

        void run() {

            uint32_t ip, prev_ip = vm.get_reg(RIP).as_u32;
            Trace* trace;

            for (;;) {

                uint32_t inst = this->vm.fetch();
                ip = vm.get_reg(RIP).as_u32;
                if (ip < prev_ip) {
                    profiler.register_header(ip);
                }
                profiler.profile(ip);
                trace = tracer.get_trace(ip);
                if (trace != nullptr && trace->get_func() != nullptr) {
                    //trace->get_func()();
                }
                else if (trace != nullptr && tracer.get_active_traces().empty()){
                    compile_trace(trace);
                    //trace->get_func()();
                }

                else if (profiler.is_hot(ip)) {
                    if (tracer.get_tracing())
                        tracer.pop_trace();
                    else {
                        auto trace = std::make_unique<Trace>();
                        tracer.push_trace(trace.get());
                        tracer.map_trace(ip, std::move(trace));
                        tracer.set_tracing(true);
                    }
                }


                else if (ip > prev_ip + 1 && tracer.get_tracing()) {
                    trace = tracer.peek_top_trace();
                    auto st = std::make_unique<Trace>();
                    trace->push_path(std::move(st));
                    tracer.capture_inst(inst);
                    tracer.push_trace(st.get());
                }

                if (tracer.get_tracing())
                    tracer.capture_inst(inst);

                prev_ip = ip;
                int interp_res = this->vm.interpret(inst);
                if (!interp_res) return;

        }
    }
        

};


#endif
