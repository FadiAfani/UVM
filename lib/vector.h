#ifndef VECTOR_H
#define VECTOR_H

#include "memory.h"
#include <stdlib.h>
#include <string.h>


typedef struct Vector{
    size_t capacity;
    size_t size;
    size_t esize;
    void* arr;
}Vector;

void append(Vector* vector, void* src, size_t n, size_t arr_esize);

#define INIT_VECTOR_CAP 4

#define CAST_VECTOR(vector, type) ((type*) vector.arr)
#define INDEX_VECTOR(vector, type, i) (CAST_VECTOR(vector, type)[i])


#define APPEND_MULT(vec, src, nbytes, src_esize) ({ \
    if (vec.capacity - vec.size <= nbytes) { \
        REALLOC_VECTOR(vec, vec.esize); \
    } \
    char* arr = (char*) vec.arr; \
    memcpy(arr + (vec.size * src_esize), src, nbytes * src_esize); \
    vec.size += nbytes; \
})

#define APPEND(vec, src, src_esize) (APPEND_MULT(vec, src, 1, src_esize))

#define INSERT_AT(vector, elem, type, idx) ({ \
    if (vector.size >= vector.capacity) { \
        REALLOCATE(vector.arr, vector.capacity, sizeof(type)); \
        vector.capacity *= 2; \
    } \
    if (idx < vector.capacity) { \
        ((type*) vector.arr)[idx] = elem; \
        vector.size++; \
    } \
})

#define REALLOC_VECTOR(vector) ({ \
    REALLOCATE(vector.arr, vector.capacity, vector.esize); \
    vector.capacity *= SCALE_FACTOR; \
})

#define INIT_VECTOR(vector, es) ({ \
    vector.capacity = INIT_VECTOR_CAP; \
    vector.size = 0; \
    vector.esize = es; \
    ALLOCATE(vector.arr, es, INIT_VECTOR_CAP); \
})


#define ALLOC_VECTOR(ptr) (ALLOCATE(ptr, sizeof(Vector), 1))

#define FREE_VECTOR(vector) (free(vector.arr))


#endif
