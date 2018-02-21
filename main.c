#include "common.h"
#include "chunk.c"
#include "debug.c"

int main(int argc, const char *argv[])
{
    Chunk c;
    chunk_init(&c);

    chunk_write_constant(&c, 1.2, 123);
    chunk_write_constant(&c, 3.4, 124);
    chunk_write_constant(&c, 5.6, 124);
    chunk_write(&c, OP_RETURN, 125);
    chunk_disassemble(&c, "test chunk");
    chunk_free(&c);
    return 0;
}
