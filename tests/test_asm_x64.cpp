#include <criterion/criterion.h>
#include <criterion/internal/test.h>
#include <cstring>
#include "../include/assembler_x64.h"

X64::Assembler assembler;
X64::Register reg(X64::RAX, 8);

void setup(void) {
    assembler.init_mmem();
}

TestSuite(test_x64, .init=setup);

Test(test_x64, test_add_imm) {
    assembler.add(reg, (uint32_t) 100);
    uint8_t mc[] = {
        REX(1,0,0,0),
        0x81,
        MOD_BYTE(3, 0, reg.encoding),
        100,0,0,0
    };
    cr_expect_eq(assembler.get_buf_size(), sizeof(mc));
    cr_expect(std::memcmp(assembler.get_buf(), mc, sizeof(mc)) == 0);

}

Test(test_x64, test_mov_imm) {
    assembler.mov(reg, (uint64_t) 100);
    uint8_t mc[] = {
        REX(1,0,0,0),
        static_cast<uint8_t>(0xb8 + reg.encoding),
        100,0,0,0,0,0,0,0
    };
    cr_expect_eq(assembler.get_buf_size(), sizeof(mc));
    cr_expect(std::memcmp(assembler.get_buf(), mc, sizeof(mc)) == 0);

}
