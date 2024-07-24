#include "../lib/vector.h"
#include "../include/memory.h"

void free_vector(Vector* vec) {
    free(vec->arr);
    free(vec);
}

void append_arr_to_vector(Vector* vec, void* src, size_t n, size_t src_esize) {
    if (vec == NULL || src == NULL)
        return;

    if (vec->esize != src_esize) {
        fprintf(stderr, "concat_vectors: vectors have different esize fields");
        exit(EXIT_FAILURE);
    }

    if (vec->capacity - vec->size < n) {
        REALLOC_VECTOR((*vec));
    }

    char* arr = vec->arr;
    memcpy(arr + vec->size, src, n * src_esize);
    vec->size += n;
    
}

void inline concat_vectors(Vector* va, Vector* vb) {
    if (va != NULL && vb != NULL) {
        append_arr_to_vector(va, vb->arr, vb->size, vb->esize);
    }

}


