#pragma once
#include "buf.h"
#include "chunk.c"
#include "common.h"
#include "debug.c"

#include <stdio.h>

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} VMInterpretResult;

typedef struct {
    Chunk *chunk;
    byte *ip;
    Value *stack; // stretchy buffer
} VM;

static void vm_reset_stack(VM *vm)
{
    buf_clear(vm->stack);
}

void vm_init(VM *vm)
{
    buf_reserve(vm->stack, 256);
    vm_reset_stack(vm);
}

void vm_free(VM *vm)
{
    buf_free(vm->stack);
}

static VMInterpretResult vm_run(VM *vm)
{
#define PUSH(value) (buf_push(vm->stack, value))
#define POP() (*buf_pop(vm->stack))
#define NEXT() (*vm->ip++)
#define READ_CONSTANT() (vm->chunk->constants[NEXT()])

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        // printf("          ");
        for (Value *it = vm->stack; it != buf_end(vm->stack); ++it) {
            printf("[ ");
            print_value(*it);
            printf(" ]");
        }
        printf("\n");
        instr_disassemble(vm->chunk, (int)(vm->ip - vm->chunk->code));
#endif
        byte instr;
        switch (instr = NEXT()) { // clang-format off
            case OP_CONSTANT: { PUSH(READ_CONSTANT()); break; }
            case OP_ADD: { PUSH(POP() + POP()); break; }
            case OP_SUB: { PUSH(-POP() + POP()); break; }
            case OP_MUL: { PUSH(POP() * POP()); break; }
            case OP_DIV: { Value y = POP(); Value x = POP(); PUSH(x / y); break; }
            case OP_NEGATE: { PUSH(-POP()); break; }
            case OP_RETURN: { print_value(POP()); printf("\n"); return INTERPRET_OK; }
        } // clang-format on
    }

#undef PUSH
#undef POP
#undef NEXT
#undef READ_CONSTANT
}

VMInterpretResult vm_interpret(VM *vm, Chunk *chunk)
{
    vm->chunk = chunk;
    vm->ip = vm->chunk->code;
    return vm_run(vm);
}
