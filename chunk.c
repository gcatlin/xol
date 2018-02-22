#ifndef _CHUNK_C_
#define _CHUNK_C_

#include "common.h"
#include "memory.c"
#include "value.c"

typedef enum {
    OP_CONSTANT,
    OP_CONSTANT_X,
    OP_RETURN,
} OpCode;

static int InstrSize[] = {
    [OP_CONSTANT]   = 2,
    [OP_CONSTANT_X] = 4,
    [OP_RETURN]     = 1,
};

typedef struct {
    int line;
    int byte_offset;
} CodeLine;

typedef struct {
    int len;
    int cap;
    int lines_len; // for `lines` and `offsets`
    int lines_cap; // for `lines` and `offsets`
    byte *code;
    int *lines;    // array of line numbers
    int *offsets;  // array of byte offsets at the start of each line
    ValueArray constants;
} Chunk;

void chunk_init(Chunk *c)
{
    c->len = 0;
    c->cap = 0;
    c->code = NULL;
    c->lines_len = 0;
    c->lines_cap = 0;
    c->lines = NULL;
    c->offsets = NULL;
    value_array_init(&c->constants);
}

int chunk_add_constant(Chunk *c, const Value v)
{
    value_array_write(&c->constants, v);
    return c->constants.len - 1;
}

void chunk_free(Chunk *c)
{
    FREE_ARRAY(byte, c->code, c->cap);
    FREE_ARRAY(int, c->lines, c->lines_cap);
    FREE_ARRAY(int, c->offsets, c->lines_cap);
    value_array_free(&c->constants);
    chunk_init(c);
}

int chunk_get_line(const Chunk *c, const int offset)
{
    bool found = false;
    int low = 0;
    int mid;
    int high = c->lines_len - 1;
    while (low <= high) {
        mid = (low + high) / 2;
        if (offset < c->offsets[mid]) { high = mid - 1; }
        else if (offset > c->offsets[mid]) { low = mid + 1; }
        else { found = true; break; }
    }
    if (!found) { mid -= (c->offsets[mid] < offset) ? 0 : 1; }
    return c->lines[mid];
}

void chunk_write(Chunk *c, byte b, int line)
{
    if (c->cap < c->len + 1) {
        int old_cap = c->cap;
        c->cap = GROW_CAPACITY(old_cap);
        c->code = GROW_ARRAY(c->code, byte, old_cap, c->cap);
    } if (c->lines_len == 0 || c->lines[c->lines_len - 1] != line) {
        if (c->lines_cap < c->lines_len + 1) {
            int old_cap = c->lines_cap;
            c->lines_cap = GROW_CAPACITY(old_cap);
            c->lines = GROW_ARRAY(c->lines, int, old_cap, c->lines_cap);
            c->offsets = GROW_ARRAY(c->offsets, int, old_cap, c->lines_cap);
        }
        c->lines[c->lines_len] = line;
        c->offsets[c->lines_len] = c->len;
        c->lines_len++;
    }
    c->code[c->len] = b;
    c->len++;
}

void chunk_write_constant(Chunk *c, Value v, int line)
{
    int constant = chunk_add_constant(c, v);

    // if (c->constants.len <= 256) {
    if (c->constants.len <= 1) {
        chunk_write(c, OP_CONSTANT, line);
        chunk_write(c, constant, line);
        return;
    }
    chunk_write(c, OP_CONSTANT_X,           line);
    chunk_write(c, constant & 0xFF,         line);
    chunk_write(c, (constant >> 8) & 0xFF,  line);
    chunk_write(c, (constant >> 16) & 0xFF, line);
}

#endif
