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

void vm_push(VM *vm, Value value)
{
    buf_push(vm->stack, value);
}

Value vm_pop(VM *vm)
{
    return *buf_pop(vm->stack);
}

static VMInterpretResult vm_run(VM *vm)
{
#define READ_BYTE() (*vm->ip++)
#define READ_CONSTANT() (vm->chunk->constants[READ_BYTE()])
#define BINARY_OP(op) \
    do { \
      double b = vm_pop(vm); \
      double a = vm_pop(vm); \
      vm_push(vm, a op b); \
    } while (false)

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
        switch (instr = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                vm_push(vm, constant);
                break;
            }
            case OP_ADD: { BINARY_OP(+); break; }
            case OP_SUB: { BINARY_OP(-); break; }
            case OP_MUL: { BINARY_OP(*); break; }
            case OP_DIV: { BINARY_OP(/); break; }
            case OP_NEGATE: { vm_push(vm, -vm_pop(vm)); break; }
            case OP_RETURN: {
                print_value(vm_pop(vm));
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }

#undef BINARY_OP
#undef READ_CONSTANT
#undef READ_BYTE
}

VMInterpretResult vm_interpret(VM *vm, Chunk *chunk)
{
    vm->chunk = chunk;
    vm->ip = vm->chunk->code;

    VMInterpretResult result = vm_run(vm);
    return result;
}
