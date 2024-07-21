#include "../lib/vector.h"
#include "../include/memory.h"

void free_vector(Vector* vec) {
    free(vec->arr);
    free(vec);
}

void append(Vector* vec, void* src, size_t n, size_t arr_esize) {
    if (vec == NULL || src == NULL) {
        fprintf(stderr, "append: vector/src is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (vec->capacity - vec->size < n) {
        REALLOC_VECTOR((*vec));
    }

    char* arr = vec->arr;
    memcpy(arr + vec->size, src, n * arr_esize);
    vec->size += n;
    
}





