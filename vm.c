#pragma once

#include "common.h"
#include "buf.h"

#include "chunk.c"
#include "compiler.c"
#include "debug.c"

static void vm_reset_stack(VM *vm)
{
    buf_clear(vm->stack);
}

static void vm_init(VM *vm)
{
    buf_reserve(vm->stack, 256);
    vm_reset_stack(vm);
}

static void vm_free(VM *vm)
{
    buf_free(vm->stack);
}

static VMInterpretResult vm_run(VM *vm)
{
#define PUSH(value) (buf_push(vm->stack, value))
#define POP() (*buf_pop(vm->stack))
#define NEXT() (*vm->ip++)
#define READ_CONSTANT() (vm->chunk->constants[NEXT()])
#define READ_CONSTANT_X(b0, b1, b2) \
    ((Value)vm->chunk->constants[b0 << 0 | b1 << 8 | b2 << 16])

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        // Print stack
        if (!buf_empty(vm->stack)) {
            fputs("\t[ ", stdout);
            for (Value *it = vm->stack; it != buf_end(vm->stack); ++it) {
                print_value(*it);
                fputs(" ", stdout);
            }
            fputs("]\n", stdout);
        }
        instr_disassemble(vm->chunk, (int)(vm->ip - vm->chunk->code));
#endif
        byte instr;
        switch (instr = NEXT()) { // clang-format off
            case OP_CONSTANT:   { PUSH(READ_CONSTANT()); break; }
            case OP_CONSTANT_X: { PUSH(READ_CONSTANT_X(NEXT(), NEXT(), NEXT())); break; }
            case OP_ADD:        { PUSH(POP() + POP()); break; }
            case OP_SUB:        { PUSH(-POP() + POP()); break; }
            case OP_MUL:        { PUSH(POP() * POP()); break; }
            case OP_DIV:        { Value y = POP(); Value x = POP(); PUSH(x / y); break; }
            case OP_NEGATE:     { PUSH(-POP()); break; }
            case OP_RETURN:     { puts(""); print_value(POP()); puts(""); return INTERPRET_OK; }
        } // clang-format on
    }

#undef PUSH
#undef POP
#undef NEXT
#undef READ_CONSTANT
#undef READ_CONSTANT_X
}

static VMInterpretResult vm_interpret(VM *vm, const char *source)
{
    Chunk *chunk = calloc(1, sizeof(Chunk));
    chunk_init(chunk);

    if (!compile(source, chunk)) {
        chunk_free(chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm->chunk = chunk;
    vm->ip = vm->chunk->code;
    VMInterpretResult result = vm_run(vm);

    chunk_free(chunk);

    return result;
}
