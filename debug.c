#pragma once

#include "common.h"
#include "chunk.c"

static int InstrSize[op__count] = {
    [OP_CONSTANT]   = 2,
    [OP_CONSTANT_X] = 4,
};

static void print_value(Value v)
{
    printf("%g", v);
}

static int const_instr(const char *name, const Chunk *c, const int offset)
{
    byte byte0 = c->code[offset + 1];
    int constant = byte0;
    printf("%-16s %4d '", name, constant);
    print_value(c->constants[constant]);
    printf("'");
    return offset + 2;
}

static int const_long_instr(const char *name, const Chunk *c, const int offset)
{
    byte byte0 = c->code[offset + 1];
    byte byte1 = c->code[offset + 2];
    byte byte2 = c->code[offset + 3];
    int constant = (byte0 << 0) | (byte1 << 8) | (byte2 << 16);
    printf("%-16s %4d '", name, constant);
    print_value(c->constants[constant]);
    printf("'");
    return offset + 4;
}

static int simple_instr(const char *name, const int offset)
{
    printf("%-16s           ", name);
    return offset + 1;
}

static int unknown_instr(const byte instr, const int offset)
{
    printf("Unknown opcode: %d", instr);
    return offset + 1;
}

static int instr_disassemble(const Chunk *chunk, const int offset)
{
#define HEX "%02hhX"

    int line = chunk_get_line(chunk, offset);
    byte instr = chunk->code[offset];
    int size = InstrSize[instr] ? InstrSize[instr] : 1;

    // Instruction bytes
    printf("%06X ", offset);
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

    switch (instr) { // clang-format off
        case OP_CONSTANT:   const_instr("OP_CONSTANT", chunk, offset); break;
        case OP_CONSTANT_X: const_long_instr("OP_CONSTANT_X", chunk, offset); break;
        case OP_ADD:        simple_instr("OP_ADD", offset); break;
        case OP_SUB:        simple_instr("OP_SUB", offset); break;
        case OP_MUL:        simple_instr("OP_MUL", offset); break;
        case OP_DIV:        simple_instr("OP_DIV", offset); break;
        case OP_NEGATE:     simple_instr("OP_NEGATE", offset); break;
        case OP_RETURN:     simple_instr("OP_RETURN", offset); break;
        default:            unknown_instr(instr, offset); break;
    } // clang-format on

    return offset + size;

#undef HEX
}

static void chunk_disassemble(Chunk *c, const char *name)
{
    printf("=== %s ===\n", name);
    printf("OFFSET B0 B1 B2 B3 LINE   OPCODE\n");
    printf("------ -- -- -- -- -----  ----------------\n");
    for (int i = 0, max = buf_len(c->code); i < max;) {
        i = instr_disassemble(c, i);
        printf("\n");
    }
    printf("\n");
}
