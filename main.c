#include "chunk.c"
#include "debug.c"
#include "vm.c"

int main(int argc, const char *argv[])
{
    buf_test();

    VM *vm = calloc(1, sizeof(VM));
    vm_init(vm);

    Chunk *c = calloc(1, sizeof(Chunk));
    chunk_init(c);
    chunk_write_constant(c, 1.2, 123);
    chunk_write_constant(c, 3.4, 123);
    chunk_write(c, OP_ADD, 123);

    chunk_write_constant(c, 5.6, 123);
    chunk_write(c, OP_DIV, 123);
    chunk_write(c, OP_NEGATE, 123);
    chunk_write(c, OP_RETURN, 123);
    chunk_disassemble(c, "test chunk");

    vm_interpret(vm, c);

    chunk_free(c);
    vm_free(vm);

    return 0;
}
