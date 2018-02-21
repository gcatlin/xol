#ifndef _VALUE_C_
#define _VALUE_C_

#include "common.h"
#include "memory.c"

typedef double Value;

typedef struct {
    int len;
    int cap;
    Value *values;
} ValueArray;

void value_array_init(ValueArray *va)
{
    va->len = 0;
    va->cap = 0;
    va->values = NULL;
}

void value_array_free(ValueArray *va)
{
    FREE_ARRAY(Value, va->values, va->cap);
    value_array_init(va);
}

void value_array_write(ValueArray *va, Value v)
{
    if (va->cap < va->len + 1) {
        int old_cap = va->cap;
        va->cap = GROW_CAPACITY(old_cap);
        va->values = GROW_ARRAY(va->values, Value, old_cap, va->cap);
    }
    va->values[va->len] = v;
    va->len++;
}

void value_print(Value v)
{
    printf("%g", v);
}

#endif
