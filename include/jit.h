#ifndef JIT_H
#define JIT_H

#include "../lib/vector.h"


Vector* gen_x64(struct vm* vm, size_t addr, size_t len);
Vector* gen_arm(struct vm* vm, size_t addr, size_t len);

#endif
