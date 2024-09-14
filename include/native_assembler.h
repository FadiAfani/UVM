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
        void make_space();
        void init_mmem();
        void emit_byte(uint8_t byte);
        template<typename T>
        void emit_imm(T imm);


};


#endif
