#include "../include/jit.h"
#include <fstream>
#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <utility>
#include <vector>

int Trace::get_heat() { 
    return this->heat; 
}

exec_func Trace::get_func() { 
    return this->func; 
}

const std::vector<uint32_t>& Trace::get_bytecode() { 
    return this->bytecode; 
}

void Trace::set_path_num(int n) {
    this->path_num = n;
}

void Trace::set_func(exec_func func) {
    this->func = func; 
}

void Trace::push_inst(uint32_t inst) {
    this->bytecode.push_back(inst);
}

void Trace::push_path(Trace* trace) {
    trace->set_path_num(this->paths.size());
    this->paths.push_back(trace);
}

void Trace::inc_heat() {
    this->heat++;
}


JITCompiler::JITCompiler() {
    this->init_mmem();
}


JITCompiler::JITCompiler(ArchType arch) {
    this->target_arch = arch;
    this->init_mmem();
}

unsigned int JITCompiler::get_cpu_reg(Reg vm_reg) {
    unsigned int r;
    try {
        r = this->x64_reg.at(vm_reg);
    } catch(std::out_of_range& e) {
        std::cout << "get_cpu_reg: unordered_map.at" << std::endl;

    }

    return r;
}

bool JITCompiler::get_is_tracing() { return this->is_tracing; }

void JITCompiler::set_target_arch(ArchType arch) {
    this->target_arch = arch;
}

void JITCompiler::dump_output_into_file(const char* fn) {
    std::ofstream fd;
    fd.open(fn, std::ostream::out);
    const char* buff = reinterpret_cast<const char*>(this->buf);
    fd.write(buff, this->buf_size);
    fd.close();
}


void JITCompiler::gen_x64(const std::vector<uint32_t>& bytecode) {

    unsigned int ra;
    unsigned int rb;
    unsigned int rd;

    for (uint32_t inst : bytecode) {
        uint8_t opcode = GET_OPCODE(inst);
        ra = this->get_cpu_reg(GET_RA(inst));
        rb = this->get_cpu_reg(GET_RB(inst));
        rd = this->get_cpu_reg(GET_RD(inst));
        //printf("opcode: %d, ra: %d, rb: %d, rd: %d\n", opcode, GET_RA(inst), GET_RB(inst), GET_RD(inst));


        switch(opcode) {
            case OP_MOV:
            {
                uint8_t mc[] = {
                    REX(1,0,0,0),
                    0x89,
                    MOD_BYTE(0x03, ra, rd) 
                };

                this->emit(mc, sizeof(mc));
                break;
            }
            case OP_MOVI:
            {
                uint32_t imm = GET_IMM19(inst);
                uint8_t mc[] = {
                    REX(1,0,0,0),
                    0xc7,
                    MOD_BYTE(0x03, 0, rd),
                    0,
                    0,
                    0,
                    0
                };
                *(uint32_t*)(mc + 3) = imm;
                this->emit(mc, sizeof(mc));
                break;

            }

            case OP_ADDI:
            {
                uint32_t imm = GET_IMM14(inst);
                uint8_t mc[] = {
                    REX(1, 0, 0, 0),
                    0x81,
                    MOD_BYTE(0x03, 0, ra),
                    0,
                    0,
                    0,
                    0,
                    REX(1, 0, 0, 0),
                    0x89,
                    MOD_BYTE(0x03, ra, rd) 
                };
                *(uint32_t*)(mc + 3) = imm;
                this->emit(mc, sizeof(mc));
                break;
            }
            case OP_SUBI:
            {
                uint32_t imm = GET_IMM19(inst);
                uint8_t mc[] = {
                    REX(1, 0, 0, 0),
                    0x81,
                    MOD_BYTE(0x03, 0x05, ra), 
                    0,
                    0,
                    0,
                    0,
                    REX(1,0,0,0),
                    0x89,
                    MOD_BYTE(0x03, ra, rd) 
                };
                *(uint32_t*)(mc + 3) = imm;
                this->emit(mc, sizeof(mc));
                break;
            }
            case OP_ADD:
            {
                uint8_t mod1 = 0x03 << 6 | ra << 3 | rb; 
                uint8_t mod2 = 0x03 << 6 | rb << 3 | rd;

                uint8_t mc[] = { 
                    0x48,
                    0x01,
                    mod1,
                    0x48, 
                    0x89, 
                    mod2
                }; 
                this->emit(mc, sizeof(mc));
                break;
            }
            case OP_SUB:
            {

                uint8_t mod1 = 0x03 << 6 | ra  << 3 | rb;
                uint8_t mod2 = 0x03 << 6 | rb << 3 | rd; 

                uint8_t mc[] = { 
                    0x48,
                    0x29,
                    mod1,
                    0x48, 
                    0x89, 
                    mod2
                }; 
                this->emit(mc, sizeof(mc));
                break;
            }
            case OP_MULT:
            {   

                uint8_t mod1 = 0x03 << 6 | ra << 3 | rb; 
                uint8_t mod2 = 0x03 << 6 | rb << 3 | rd; 

                uint8_t mc[] = { 
                    0x48,
                    0x0F,
                    0xAF,
                    mod1,
                    0x48, 
                    0x89, 
                    mod2
                }; 
                this->emit(mc, sizeof(mc));
                break;
            }
            case OP_DIV:
            {
                /* {div rd ra rb}
                 * mov rax, ra
                 * idiv rb
                 * mov rd, rax
                 * */

                uint8_t mod1 = 0x03 << 6 | ra << 3; // mov rax, ra
                uint8_t mod2 = 0x03 << 6 | 0x07 << 3 | rb; // idiv rb
                uint8_t mod3 = 0x03 << 6 | rd; // mov rd, rax
                uint8_t mc[] = {
                    0x48,
                    0x89,
                    mod1,
                    0x48,
                    0x33,
                    0x03 << 6 | 0x02 << 3 | 0x02, // xor rdx, rdx
                    0x48,
                    0xf7,
                    mod2,
                    0x48,
                    0x89,
                    mod3
                };

                this->emit(mc, sizeof(mc));
                break;
            }
            case OP_RET:
            {
                uint8_t mc[] = {0xc3};
                this->emit(mc, sizeof(mc));
                break;
            }
            case OP_HALT:
            {
                uint8_t mc[] = {0xf4};
                this->emit(mc, sizeof(mc));
                break;
            }
            case OP_PUSH:
            {
                uint8_t mc[] = {
                    0xff,
                    MOD_BYTE(0x3, 0x6, rd),
                };
                this->emit(mc, sizeof(mc));
                break;
            }
            case OP_CALL:
            {
                uint8_t mc[] = {
                    0xff,
                    MOD_BYTE(0x0, 0x2, rd),
                };
                this->emit(mc, sizeof(mc));
                break;
            }
            case OP_CMP:
            {
                uint8_t mc[] = {
                    REX(1, 0, 0, 0),
                    0x3b,
                    MOD_BYTE(0x3, rd, ra), 
                };
                this->emit(mc, sizeof(mc));
                break;

            }

            case OP_LDR:
            {
                /* { ldr rd [ra + disp] }
                 * mov rd, vm->memory
                 * add ra, rd
                 * mov rd, [ra + disp]
                 * */
                uint8_t dst = 0xb8 + this->get_cpu_reg(GET_RD(inst));
                int32_t disp = GET_IMM14(inst);
                uint8_t mc[] = {
                    REX(1,0,0,0),
                    dst,
                    0,0,0,0,0,0,0,0,
                    REX(1,0,0,0),
                    0x03,
                    MOD_BYTE(0x03, ra, rd), 
                    REX(1,0,0,0),
                    0x8b,
                    MOD_BYTE(0x2, rd, ra), // 4-byte displacement
                    0,0,0,0
                };
                *(uint64_t*) (mc + 2) = reinterpret_cast<uint64_t>(this->buf);
                *(int32_t*) (mc + 16) = disp;
                this->emit(mc, sizeof(mc));
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
                uint8_t mc[] = {
                    REX(1,0,0,0),
                    0xb8 + 0x01,
                    0,0,0,0,0,0,0,0,
                    REX(1,0,0,0),
                    0x03,
                    MOD_BYTE(0x03, ra, 0x01),
                    REX(1,0,0,0),
                    0x89,
                    MOD_BYTE(0x2, rd, ra), // 4-byte displacement
                    0,0,0,0
                };
                *(uint64_t*) (mc + 2) = reinterpret_cast<uint64_t>(this->buf);
                *(int32_t*) (mc + 16) = disp;
                this->emit(mc, sizeof(mc));
                break;
            }
            case OP_JB:
                break;
            case OP_JBE:
                break;
            case OP_JL:
                break;
            case OP_JLE:
                break;
            case OP_JE:
                break;
            case OP_JNE:
                break;
            case OP_JMP:
                break;

            default:
                fprintf(stderr, "Not Implemented\n");
                exit(EXIT_FAILURE);

        }
    }
    //free_hash_table(&jmp_pts);
}

void transfer_reg_x64(JITCompiler* jit, VM* vm, unsigned int cpu_reg, Reg vm_reg, bool to_cpu) {

    uint8_t opcode = 0x89;
    if (to_cpu) 
        opcode = 0x8b;

    /* mov rax, imm64
     * mov cpu_reg, [rax]
     * */
    uint8_t mc[] = {
        REX(1,0,0,0),
        0xb8,
        0,0,0,0,0,0,0,0,
        REX(1,0,0,0),
        opcode,
        MOD_BYTE(0, cpu_reg, 0)
    };
    *(uint64_t*) (mc + 2) = vm->get_reg(vm_reg).as_u64;
     jit->emit(mc, sizeof(mc));
}


/* simply transfers the entire state regardless of changes
 * TODO: this method should only transfer modified state 
 * */
void JITCompiler::transfer_reg_state(VM* vm, bool to_cpu, const RegTransferFunc tfunc) {

    for (auto p : this->x64_reg) {
        tfunc(this, vm, p.second, p.first, to_cpu);
    }

}

Trace* JITCompiler::get_trace(uint32_t ip) {
    Trace* t;
    try {
        t = this->trace_map.at(ip);
    } catch(std::out_of_range& e) {
        return nullptr;
    }

    return t;
}

void JITCompiler::map_trace(uint32_t ip, Trace* trace) {
    std::pair<int, Trace*> p (ip, trace);
    this->trace_map.insert(p);

}
bool JITCompiler::compile_trace(VM* vm, Trace* trace) {
    if (trace == nullptr || vm == nullptr)
        return false;


    //TODO: buf is not dynamic !!! 
    trace->push_inst(OP_RET << 24);
    trace->set_func(reinterpret_cast<exec_func>(this->buf + this->buf_size));
    this->gen_x64(trace->get_bytecode());
    this->dump_output_into_file("binary_dump");
    
    return true;
    
}

void JITCompiler::emit(uint8_t* buff, size_t len) {

    if (buff != nullptr) {
        memcpy(this->buf + this->buf_size, buff, len);
        this->buf_size += len;
    }
}


void JITCompiler::profile(VM* vm, uint32_t inst) {
    uint32_t prev_ip = vm->get_reg(RIP).as_u32;
    uint32_t ip;
    Trace* trace = nullptr;

    switch(GET_OPCODE(inst)) {
        case OP_JB:
        case OP_JE:
        case OP_JL:
        case OP_JBE:
        case OP_JLE:
        case OP_JNE:
        case OP_JMP:
            vm->interpret(inst);
            ip = vm->get_reg(RIP).as_u32;
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
                
            } else {
                /* condition unsatisfied */
                trace = new Trace();
                Trace* top = this->peek_top_trace();
                if (top != nullptr) {
                    trace->set_path_num(top->get_bytecode().size());
                    top->push_path(trace);
                    this->push_trace(trace);
                    this->is_tracing = true;
                }

            }

            break;
    }

}

void JITCompiler::record_inst(VM* vm,  uint32_t inst) {
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

void JITCompiler::init_mmem() {
    this->buf = static_cast<uint8_t*>(
            mmap(NULL, 
                4096,
                PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_ANON | MAP_PRIVATE,-1, 0)
            );
}

void JITCompiler::push_trace(Trace* trace) {
    this->active_traces.push(trace);
}

Trace* JITCompiler::pop_trace() {
    Trace* t = nullptr;
    if (!this->active_traces.empty()) {
        t = this->active_traces.top();
        this->active_traces.pop();
    } 
    return t;
}

Trace* JITCompiler::peek_top_trace() {
    Trace* t = nullptr;
    if (!this->active_traces.empty()) {
        t = this->active_traces.top();
    }
    return t;
}
