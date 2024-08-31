#include "../include/jit.h"
#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <utility>
#include <vector>



JITCompiler::JITCompiler(): x64_reg(
    {
        {R0, 0},
        {R1, 3},
        {R2, 1},
        {R3, 2},
        {R4, 6},
        {R5, 7},
        {R6, 5},
        {R7, 4}
    }) 

{

}


JITCompiler::JITCompiler(ArchType arch): x64_reg(
    {
        {R0, 0},
        {R1, 3},
        {R2, 1},
        {R3, 2},
        {R4, 6},
        {R5, 7},
        {R6, 5},
        {R7, 4}
    }) 

{
    this->target_arch = arch;
}

unsigned int JITCompiler::get_cpu_reg(Reg vm_reg) {
    uint8_t r;
    try {
        r = this->x64_reg.at(vm_reg);
    } catch(std::out_of_range& e) {
        std::cout << e.what() << std::endl;
    }

    return r;
}

Trace* JITCompiler::get_tp() { return this->tp; }
bool JITCompiler::get_is_tracing() { return this->is_tracing; }

void JITCompiler::set_target_arch(ArchType arch) {
    this->target_arch = arch;
}

void JITCompiler::dump_output_into_file(const char* fn) {

}


void JITCompiler::gen_x64(VM* vm, const std::vector<uint32_t>& bytecode) {
    uint32_t inst;
    unsigned int ra;
    unsigned int rb;
    unsigned int rd;

    //TODO: fail when addr/len > MEM_SIZE
    for (size_t i = 0; i < bytecode.size(); i++) {
        uint8_t opcode = GET_OPCODE(inst);
        ra = this->get_cpu_reg(GET_RA(inst));
        rb = this->get_cpu_reg(GET_RB(inst));
        rd = this->get_cpu_reg(GET_RD(inst));


        switch(opcode) {
            case OP_MOV:
            {
                uint8_t mc[] = {
                    REX(1,0,0,0),
                    0x89,
                    MOD_BYTE(0x03, ra, rd) 
                };

                this->append_code(mc, sizeof(mc));
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
                    0
                };
                *(uint32_t*)(mc + 3) = imm;
                this->append_code(mc, sizeof(mc));
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
                this->append_code(mc, sizeof(mc));
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
                this->append_code(mc, sizeof(mc));
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
                this->append_code(mc, sizeof(mc));
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
                this->append_code(mc, sizeof(mc));
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
                this->append_code(mc, sizeof(mc));
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

                this->append_code(mc, sizeof(mc));
                break;
            }
            case OP_RET:
            {
                uint8_t mc[] = {0xc3};
                this->append_code(mc, sizeof(mc));
                break;
            }
            case OP_HALT:
            {
                uint8_t mc[] = {0xf4};
                this->append_code(mc, sizeof(mc));
                break;
            }
            case OP_PUSH:
            {
                uint8_t mc[] = {
                    0xff,
                    MOD_BYTE(0x3, 0x6, rd),
                };
                this->append_code(mc, sizeof(mc));
                break;
            }
            case OP_CALL:
            {
                uint8_t mc[] = {
                    0xff,
                    MOD_BYTE(0x0, 0x2, rd),
                };
                this->append_code(mc, sizeof(mc));
                break;
            }
            case OP_CMP:
            {
                uint8_t mc[] = {
                    REX(1, 0, 0, 0),
                    0x3b,
                    MOD_BYTE(0x3, rd, ra), 
                };
                this->append_code(mc, sizeof(mc));
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
                *(uint64_t*) (mc + 2) = reinterpret_cast<uint64_t>(this->mmem);
                *(int32_t*) (mc + 16) = disp;
                this->append_code(mc, sizeof(mc));
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
                *(uint64_t*) (mc + 2) = reinterpret_cast<uint64_t>(this->mmem);
                *(int32_t*) (mc + 16) = disp;
                this->append_code(mc, sizeof(mc));
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


void JITCompiler::transfer_reg_x64(VM* vm, unsigned int cpu_reg, Reg vm_reg, bool to_cpu) {

    uint8_t opcode = 0x89;
    if (to_cpu) {
        opcode = 0x8b;
    }

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
     this->append_code(mc, sizeof(mc));
}

void JITCompiler::transfer_reg_state(VM* vm, bool to_cpu, const RegTransferFunc tfunc) {

}

Trace* JITCompiler::get_trace(uint32_t ip) {
    Trace* t = nullptr;
    try {
        t = this->trace_map.at(ip);
    } catch(std::out_of_range& e) {
        std::cout << e.what() << std::endl;
    }

    return t;
}

void JITCompiler::map_trace(uint32_t ip, Trace* trace) {
    std::pair<int, Trace*> p (ip, trace);
    this->trace_map.insert(p);

}
void JITCompiler::compile_trace(VM* vm, Trace* trace) {
    if (trace == nullptr)
        return;

    //TODO: mmem is not dynamic !!! 
    trace->bytecode.push_back(OP_RET << 24);
    trace->func = reinterpret_cast<exec_func>(this->mmem + this->mmem_size);
    gen_x64(vm, trace->bytecode);
    
}



void JITCompiler::append_code(uint8_t* buff, size_t len) {

    if (buff != NULL) {
        memcpy(this->mmem + this->mmem_size, buff, len);
        this->mmem_size += len;
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
                    trace->heat++;
                    this->is_tracing = false;
                } else {
                    trace = new Trace();
                    this->is_tracing = true;
                }
                this->tp = trace;
            }
    }

}

void JITCompiler::record_inst(VM* vm, uint32_t inst) {
    if (this->tp == NULL || !this->is_tracing) 
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
            this->tp->bytecode.push_back(inst);
            break;
    }
}

void JITCompiler::init_mmem() {
    this->mmem = static_cast<uint8_t*>(
            mmap(NULL, 
                4096,
                PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_ANON | MAP_PRIVATE,-1, 0)
            );
}

void JITCompiler::get_mod_regs(uint32_t inst) {
    Reg rd;
    switch(GET_OPCODE(inst)) {
        /* instructions that use the rd register */
        case OP_ADD:
        case OP_ADDI:
        case OP_FADD:
        case OP_SUB:
        case OP_SUBI:
        case OP_FSUB:
        case OP_MOV:
        case OP_MOVI:
        case OP_MULT:
        case OP_FMULT:
        case OP_DIV:
        case OP_FDIV:
        case OP_CMP:
        case OP_FCMP:
        case OP_STR:
            rd = GET_RD(inst);
            this->tp->mod_regs.push(rd);
            break;
    }

}

bool JITCompiler::get_native_exec() { return this->native_exec; }

