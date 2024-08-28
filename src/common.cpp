#include "../include/common.h"

void free_trace(Trace* trace) {
    FREE_VECTOR(trace->bytecode);
    free(trace);
}

