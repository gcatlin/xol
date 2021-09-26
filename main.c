#include "common.h"
#include "buf.h"
#include "vm.c"

static size_t fsize(FILE *stream)
{
    fseek(stream, 0L, SEEK_END);
    long size = ftell(stream);
    rewind(stream);
    return size;
}

static int read_file(char *restrict buf, const char *restrict path)
{
    FILE *file = fopen(path, "rb");
    if (!file) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(ERR_FILE);
    }

    size_t sz = fsize(file);
    buf_reserve(buf, (int)(sz + 1)); // extra byte for NUL
    size_t bytes_read = fread(buf, sizeof(char), sz, file);
    if (bytes_read < sz) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        fclose(file);
        exit(ERR_FILE);
    }
    buf[bytes_read] = '\0';

    fclose(file);
    return (int)bytes_read;
}

static void eval_file(VM *vm, const char *path)
{
    char *source = NULL;
    buf_reserve(source, 16384);
    read_file(source, path);
    VMInterpretResult result = vm_interpret(vm, source).result;

    if (result == INTERPRET_COMPILE_ERROR) exit(ERR_COMPILE);
    if (result == INTERPRET_RUNTIME_ERROR) exit(ERR_RUNTIME);
}

static void repl(VM *vm)
{
    char line[1024];
    for (;;) {
        fputs(ANSI_BOLD "xol> " ANSI_RESET, stdout);
        if (!fgets(line, sizeof(line), stdin)) {
            fputs(ANSI_RESET "\n", stdout);
            break;
        }

        vm_interpret(vm, line);
    }
}

int main(int argc, const char *argv[])
{
    buf_test();
    vm_test();

    VM *vm = calloc(1, sizeof(VM));
    vm_init(vm);

    switch (argc) {
        case 1:  { repl(vm); break; }
        case 2:  { eval_file(vm, argv[1]); break; }
        default: { fputs("Usage: xol [path]\n", stderr); exit(ERR_USAGE); }
    }

    vm_free(vm);

    return 0;
}
