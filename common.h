#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION

#define ANSI_RESET     "\x1b[0m"
#define ANSI_BOLD      "\x1b[1m"
#define ANSI_FG_RED    "\x1b[31m"
#define ANSI_FG_GREEN  "\x1b[32m"
#define ANSI_FG_YELLOW "\x1b[33m"
#define ANSI_FG_BLUE   "\x1b[34m"
#define ANSI_FG_CYAN   "\x1b[36m"

#define ERR_USAGE   64
#define ERR_COMPILE 65
#define ERR_RUNTIME 70
#define ERR_FILE    74

#define countof(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

typedef uint8_t byte;
typedef double Value;

typedef enum {
    OP_CONSTANT,
    OP_CONSTANT_X,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_NEGATE,
    OP_RETURN,
    op__count,
} OpCode;

typedef struct {
    byte  *code;
    int   *lines;     // array of line numbers
    int   *offsets;   // array of byte offsets at the start of each line
    Value *constants;
} Chunk;

typedef enum {
    TOKEN_NONE,

    // Punctuation
    TOKEN_BANG, TOKEN_BANG_EQUAL, TOKEN_COMMA, TOKEN_DOT, TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL, TOKEN_GREATER, TOKEN_GREATER_EQUAL, TOKEN_LEFT_BRACE,
    TOKEN_LEFT_PAREN, TOKEN_LESS, TOKEN_LESS_EQUAL, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_RIGHT_BRACE, TOKEN_RIGHT_PAREN, TOKEN_SEMICOLON, TOKEN_SLASH,
    TOKEN_STAR,

    // Literals
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    // Keywords
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE, TOKEN_FOR, TOKEN_FN,
    TOKEN_IF, TOKEN_NIL, TOKEN_OR, TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER,
    TOKEN_THIS, TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

    TOKEN_COMMENT, TOKEN_ERROR, TOKEN_EOF
} TokenType;

typedef struct {
    TokenType  type;
    int        length;
    int        line;
    const char *start;
} Token;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . () []
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)();

typedef struct {
    Token previous;
    Token current;
    bool  had_error;
    bool  panic_mode;
} Parser;

typedef struct {
    ParseFn    prefix;
    ParseFn    infix;
    Precedence precedence;
} ParseRule;

typedef struct {
    const char *start;   // token
    const char *current; // cursor
    int         line;
} Scanner;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} VMInterpretResult;

typedef struct {
    Chunk *chunk;
    byte  *ip;
    Value *stack; // stretchy buffer
} VM;


