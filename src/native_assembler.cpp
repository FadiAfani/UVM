#include <sys/mman.h>
#include "../include/native_assembler.h"

NativeAssembler::NativeAssembler() {
    this->buf = nullptr;
    this->buf_size = 0;
    this->buf_cap = 0;
}

void NativeAssembler::init_mmem() {
    this->buf = static_cast<uint8_t*>(
            mmap(nullptr, 
                PAGE_SIZE,
                PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_ANON | MAP_PRIVATE,-1, 0)
            );
    this->buf_cap = PAGE_SIZE;
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

const uint8_t* NativeAssembler::get_buf() { return this->buf; }
size_t NativeAssembler::get_buf_size() { return this->buf_size; }
size_t NativeAssembler::get_buf_cap() { return this->buf_cap; }


