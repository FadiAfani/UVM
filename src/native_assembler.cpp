#include <sys/mman.h>
#include "../include/native_assembler.h"

void NativeAssembler::init_mmem() {
    this->buf = static_cast<uint8_t*>(
            mmap(nullptr, 
                this->buf_cap,
                PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_ANON | MAP_PRIVATE,-1, 0)
            );
}

void NativeAssembler::make_space() {
    mremap( this->buf, 
            this->buf_size,
            this->buf_cap * SCALE_FACTOR,
            PROT_READ | PROT_WRITE | PROT_EXEC | MAP_ANON | MAP_PRIVATE
          );
        this->buf_cap *= SCALE_FACTOR;
}

void NativeAssembler::emit_byte(uint8_t byte) {
    if (this->buf_size >= this->buf_cap)
        this->make_space();
    this->buf[this->buf_size++] = byte;
}


