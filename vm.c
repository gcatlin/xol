#pragma once

#include "common.h"
#include "buf.h"

#include "chunk.c"
#include "compiler.c"
#include "debug.c"

static bool values_equal(Value a, Value b)
{
    if (a.type != b.type) return false;

    switch (a.type) {
        case VAL_NIL:    return true;
        case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
    }
}

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

static void vm_runtime_error(VM *vm, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instr = vm->ip - vm->chunk->code;
    int line = vm->chunk->lines[instr];
    fprintf(stderr, "[line %d] in script\n", line);

    vm_reset_stack(vm);
}

static VMResult vm_run(VM *vm)
{
#define IS_FALSEY(v) (IS_NIL(v) || (IS_BOOL(v) && !AS_BOOL(v)))
#define PEEK(dist) (*buf_peek(vm->stack, dist))
#define PUSH(value) (buf_push(vm->stack, value))
#define POP() (*buf_pop(vm->stack))
#define NEXT() (*vm->ip++)
#define READ_CONSTANT() (vm->chunk->constants[NEXT()])
#define READ_CONSTANT_X(b0, b1, b2) \
    (vm->chunk->constants[(b0) << 0 | (b1) << 8 | (b2) << 16])
#define BINARY_OP(TO_VAL, op)                                  \
    do {                                                       \
        if (!IS_NUMBER(PEEK(0)) || !IS_NUMBER(PEEK(1))) {      \
            vm_runtime_error(vm, "Operands must be numbers."); \
            return (VMResult){ INTERPRET_RUNTIME_ERROR, {0} }; \
        }                                                      \
                                                               \
        double b = AS_NUMBER(POP());                           \
        double a = AS_NUMBER(POP());                           \
        PUSH(TO_VAL(a op b));                                  \
    } while (false)

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
            case OP_CONSTANT:   PUSH(READ_CONSTANT()); break;
            case OP_CONSTANT_X: PUSH(READ_CONSTANT_X(NEXT(), NEXT(), NEXT())); break;
            case OP_NIL:        PUSH(NIL_VAL); break;
            case OP_FALSE:      PUSH(BOOL_VAL(false)); break;
            case OP_TRUE:       PUSH(BOOL_VAL(true)); break;
            case OP_EQ:         { Value b = POP(); Value a = POP(); PUSH(BOOL_VAL(values_equal(a, b))); } break;
            case OP_GT:         BINARY_OP(BOOL_VAL, >); break;
            case OP_LT:         BINARY_OP(BOOL_VAL, <); break;
            case OP_ADD:        BINARY_OP(NUMBER_VAL, +); break;
            case OP_SUB:        BINARY_OP(NUMBER_VAL, -); break;
            case OP_MUL:        BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIV:        BINARY_OP(NUMBER_VAL, /); break;
            case OP_NOT:        PUSH(BOOL_VAL(IS_FALSEY(POP()))); break;
            case OP_NEG:        {
                                    if (!IS_NUMBER(PEEK(0))) {
                                        vm_runtime_error(vm, "Operand must be a number.");
                                        return (VMResult){ INTERPRET_RUNTIME_ERROR, {0} };
                                    }
                                    double n = -AS_NUMBER(POP());
                                    PUSH(NUMBER_VAL(n));
                                }
                                break;
            case OP_RETURN:     {
                                    Value v = POP();
                                    puts(""); print_value(v); puts("");
                                    return (VMResult){ INTERPRET_OK, v };
                                }
            default:            assert(0 && "unreachable");
        } // clang-format on
    }

#undef IS_FALSEY
#undef PEEK
#undef PUSH
#undef POP
#undef NEXT
#undef READ_CONSTANT
#undef READ_CONSTANT_X
#undef BINARY_OP
}

static VMResult vm_interpret(VM *vm, const char *source)
{
    Chunk *chunk = calloc(1, sizeof(Chunk));
    chunk_init(chunk);

    if (!compile(source, chunk)) {
        chunk_free(chunk);
        return (VMResult){ INTERPRET_COMPILE_ERROR, {0} };
    }

    vm->chunk = chunk;
    vm->ip = vm->chunk->code;
    VMResult result = vm_run(vm);

    chunk_free(chunk);

    return result;
}

static void vm_test(void)
{
    VM *vm = calloc(1, sizeof(VM));
    vm_init(vm);
    assert(7 == AS_NUMBER(vm_interpret(vm, "(-1 + 2) * 3 - -4").value));
}
