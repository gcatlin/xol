#ifndef _MEMORY_C_
#define _MEMORY_C_

#include "common.h"

#define GROW_CAPACITY(cap) ((cap) < 8 ? 8 : (cap) * 2)

#define GROW_ARRAY(prev, type, old_cap, new_cap) \
    (type *)reallocate(prev, sizeof(type) * (old_cap), sizeof(type) * (new_cap))

#define FREE_ARRAY(type, ptr, old_len) \
    reallocate(ptr, sizeof(type) * (old_len), 0)

void *reallocate(void *prev, size_t old_size, size_t new_size) {
    return realloc(prev, new_size);
}

// typedef struct {
//     int len;
//     int cap;
//     int *data;
// } IntArray;

// void int_array_init(IntArray *ia)
// {
//     ia->len = 0;
//     ia->cap = 0;
//     ia->data = NULL;
// }

// void int_array_free(IntArray *ia)
// {
//     FREE_ARRAY(int, ia->data, ia->cap);
//     int_array_init(ia);
// }

// void int_array_add(IntArray *ia, int i)
// {
//     if (ia->cap < ia->len + 1) {
//         int old_cap = ia->cap;
//         ia->cap = GROW_CAPACITY(old_cap);
//         ia->data = GROW_ARRAY(ia->data, int, old_cap, ia->cap);
//     }
//     ia->data[ia->len] = i;
//     ia->len++;
// }

// int int_array_get(IntArray *ia, int index)
// {
//     // TODO bounds checking
//     return ia->data[index];
// }

// int int_array_last(IntArray *ia)
// {
//     return ia->data[ia->len - 1];
// }

#endif
