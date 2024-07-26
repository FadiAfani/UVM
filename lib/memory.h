#ifndef MEMORY_H
#define MEMORY_H

#include <stdio.h>

#define SCALE_FACTOR 1.5

#define CHECK_FAILED_ALLOCATION(ptr) ({\
		if (!ptr) {\
		    printf("Failure in memory allocation"); \
			exit(EXIT_FAILURE); \
		} \
})

#define ALLOCATE(ptr, esize, capacity) ({\
    ptr = malloc(esize * capacity); \
    CHECK_FAILED_ALLOCATION(ptr); \
})

#define REALLOCATE(ptr, esize, prev_size) ({\
	void* new_ptr = realloc(ptr, SCALE_FACTOR * esize * prev_size); \
    if (new_ptr == NULL) { \
		    printf("Realloc failed\n"); \
			free(ptr); \
			exit(EXIT_FAILURE); \
		} \
	ptr = new_ptr; \
})

void* reverse_memcpy(void* dest, void* src, size_t len);

#endif 
