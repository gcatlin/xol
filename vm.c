#pragma once

#include "buf.h"
#include "common.h"

#include "chunk.c"
#include "debug.c"
#include "scanner.c"
#include "token.c"

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} VMInterpretResult;

typedef struct {
    Chunk *chunk;
    byte  *ip;
    Value *stack; // stretchy buffer
} VM;

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
                fputs(", ", stdout);
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

static void vm_compile(VM *vm, const char *source)
{
    Scanner *s = calloc(1, sizeof(Scanner));
    scanner_init(s, source);

    int line = -1;
    for (;;) {
        Token token = scanner_scan_token(s);
        if (token.line != line) {
            printf("%4d ", token.line);
            line = token.line;
        } else {
            printf("   | ");
        }

        if (token.type == TOKEN_EOF) {
            puts("EOF");
            break;
        }
        switch (token.type) {
            case TOKEN_ERROR:
                printf(ANSI_FG_RED "ERROR        " ANSI_RESET " %.*s\n", token.length, token.start);
                break;
            default:
                printf("%-13s '%.*s'\n", token_type_name(token.type)+6, token.length, token.start);
        }
    }
}

static VMInterpretResult vm_interpret(VM *vm, const char *source)
{
    vm_compile(vm, source);
    return INTERPRET_OK;
}
