#ifndef NATIVE_ASSEMLBER
#define NATIVE_ASSEMLBER

#include <cstddef>
#include <cstdint>
#include <sys/mman.h>

#define PAGE_SIZE 4096
#define SCALE_FACTOR 1.5 

class NativeAssembler {
    protected:
        uint8_t* buf;
        size_t buf_size;
        size_t buf_cap;

    public:
        NativeAssembler();
        const uint8_t* get_buf();
        size_t get_buf_size();
        size_t get_buf_cap();
        void make_space();
        void init_mmem();
        void emit_byte(size_t i, uint8_t byte);
        void emit_byte(uint8_t byte);
        template<typename T>
        void emit_imm(T imm) {
            if (this->buf_cap - this->buf_size < sizeof(T)) {
                this->make_space();
            }
            *(T*)(this->buf + this->buf_size) = imm;
            this->buf_size += sizeof(T);
        }
        void shift(int64_t amount);


};


#endif
