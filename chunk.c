#pragma once

#include "common.h"
#include "buf.h"

static void chunk_init(Chunk *c)
{
    buf_reserve(c->code, 1024);
    buf_reserve(c->lines, 8);
    buf_reserve(c->offsets, 8);
    buf_reserve(c->constants, 8);
}

static int chunk_add_constant(Chunk *c, const Value v)
{
    buf_push(c->constants, v);
    return buf_len(c->constants) - 1;
}

static void chunk_free(Chunk *c)
{
    buf_free(c->code);
    buf_free(c->lines);
    buf_free(c->offsets);
    buf_free(c->constants);
    chunk_init(c);
}

static int chunk_get_line(const Chunk *c, const int offset)
{
    bool found = false;
    int low = 0;
    int mid;
    int high = buf_len(c->lines) - 1;
    while (low <= high) {
        mid = (low + high) / 2;
        if (offset < c->offsets[mid]) {
            high = mid - 1;
        } else if (offset > c->offsets[mid]) {
            low = mid + 1;
        } else {
            found = true;
            break;
        }
    }
    if (!found && offset <= c->offsets[mid]) {
        --mid;
    }
    return c->lines[mid];
}

static void chunk_write(Chunk *c, const byte *bytes, int count, int line)
{
    size_t lines_len = buf_len(c->lines);
    if (lines_len == 0 || c->lines[lines_len - 1] != line) {
        buf_push(c->lines, line);
        buf_push(c->offsets, buf_len(c->code));
    }

    byte *dest = buf_append(c->code, count);
    memcpy(dest, bytes, count);
}

static void chunk_write_constant(Chunk *c, Value v, int line)
{
    int constant = chunk_add_constant(c, v);
    if (buf_len(c->constants) <= 0xFF) {
        chunk_write(c, (byte[]){ OP_CONSTANT, constant }, 2, line);
        return;
    }
    byte bytes[] = {
        OP_CONSTANT_X,
        (constant >> 0),
        (constant >> 8),
        (constant >> 16)
    };
    chunk_write(c, bytes, 4, line);
}
