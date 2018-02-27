#ifndef _DEBUG_C_
#define _DEBUG_C_

#include "common.h"
#include "chunk.c"
#include "value.c"

#define HEX "%02hhX"
// #define HEX "0x%02hhx"

void print_value(Value v)
{
    printf("%g", v);
}

int const_instr(const char *name, const Chunk *c, const int offset)
{
    byte byte0 = c->code[offset + 1];
    int constant = byte0;
    printf("%-16s %4d '", name, constant);
    print_value(c->constants.values[constant]);
    printf("'\n");
    return offset + 2;
}

int const_long_instr(const char *name, const Chunk *c, const int offset)
{
    byte byte0 = c->code[offset + 1];
    byte byte1 = c->code[offset + 2];
    byte byte2 = c->code[offset + 3];
    int constant = (byte0 << 0) | (byte1 << 8) | (byte2 << 16);
    printf("%-16s %4d '", name, constant);
    print_value(c->constants.values[constant]);
    printf("'\n");
    return offset + 4;
}

int simple_instr(const char *name, const int offset)
{
    printf("%-16s\n", name);
    return offset + 1;
}

int unknown_instr(const byte instr, const int offset)
{
    printf("Unknown opcode: %d\n", instr);
    return offset + 1;
}

int instr_disassemble(const Chunk *chunk, const int offset)
{
    int line = chunk_get_line(chunk, offset);
    byte instr = chunk->code[offset];
    int size = InstrSize[instr]; // TODO handle invalid instr (size=1)

    // Instruction bytes
    printf("%06d ", offset);
    for (int i = 0; i < size; ++i) {
        printf(HEX " ", (byte)chunk->code[offset + i]);
    }
    for (int i = size; i < 4; ++i) {
        printf("   ");
    }

    // Line numbers
    if (offset > 0 && line == chunk_get_line(chunk, offset - 1)) {
        printf("    |  ");
    } else {
        printf("%5d  ", line);
    }

    switch (instr) {
        case OP_CONSTANT:   const_instr("OP_CONSTANT", chunk, offset); break;
        case OP_CONSTANT_X: const_long_instr("OP_CONSTANT_X", chunk, offset); break;
        case OP_RETURN:     simple_instr("OP_RETURN", offset); break;
        default:            unknown_instr(instr, offset); break;
    }
    return offset + size;
}

void chunk_disassemble(Chunk *c, const char *name)
{
    printf("=== %s ===\n", name);
    printf("OFFSET B0 B1 B2 B3 LINE   OPCODE\n");
    printf("------ -- -- -- -- -----  ----------------\n");
    for (int i = 0; i < c->len; ) {
        i = instr_disassemble(c, i);
    }
}

#endif
