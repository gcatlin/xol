#pragma once

#include "common.h"
#include "debug.c"
#include "chunk.c"
#include "scanner.c"

static void binary();
static void grouping();
static void number();
static void unary();

static Chunk   *chunk;
static Scanner scanner;
static Parser  parser;

static ParseRule parse_rules[] = {
    //                        prefix    infix    precedence
    [TOKEN_NONE]          = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_BANG]          = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_BANG_EQUAL]    = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_COMMA]         = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_DOT]           = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_EQUAL]         = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_EQUAL_EQUAL]   = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_GREATER]       = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_GREATER_EQUAL] = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_LEFT_BRACE]    = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_LEFT_PAREN]    = { grouping, NULL,    PREC_NONE   },
    [TOKEN_LESS]          = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_LESS_EQUAL]    = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_MINUS]         = { unary,    binary,  PREC_TERM   },
    [TOKEN_PLUS]          = { NULL,     binary,  PREC_TERM   },
    [TOKEN_RIGHT_BRACE]   = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_RIGHT_PAREN]   = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_SEMICOLON]     = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_SLASH]         = { NULL,     binary,  PREC_FACTOR },
    [TOKEN_STAR]          = { NULL,     binary,  PREC_FACTOR },

    [TOKEN_IDENTIFIER]    = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_STRING]        = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_NUMBER]        = { number,   NULL,    PREC_NONE   },

    [TOKEN_AND]           = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_CLASS]         = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_ELSE]          = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_FALSE]         = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_FOR]           = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_FN]            = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_IF]            = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_NIL]           = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_OR]            = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_PRINT]         = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_RETURN]        = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_SUPER]         = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_THIS]          = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_TRUE]          = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_VAR]           = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_WHILE]         = { NULL,     NULL,    PREC_NONE   },

    [TOKEN_COMMENT]       = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_ERROR]         = { NULL,     NULL,    PREC_NONE   },
    [TOKEN_EOF]           = { NULL,     NULL,    PREC_NONE   },
};

static void error_at(Token *token, const char *message)
{
    if (parser.panic_mode) {
        return;
    }
    parser.panic_mode = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing.
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.had_error = true;
}

static void error(const char *message)
{
    error_at(&parser.previous, message);
}

static void error_at_current(const char *message)
{
    error_at(&parser.current, message);
}

static void advance()
{
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanner_scan_token(&scanner);
        if (parser.current.type != TOKEN_ERROR) break;

        error_at_current(parser.current.start);
    }
}

static void consume(TokenType type, const char *message)
{
    if (parser.current.type == type) {
        advance();
        return;
    }

    error_at_current(message);
}

static void emit_byte(byte b)
{
    chunk_write(chunk, (byte[]){ b }, 1, parser.previous.line);
}

static void emit_bytes(const byte *bytes, int count)
{
    chunk_write(chunk, bytes, count, parser.previous.line);
}

static void emit_bytes2(const byte b1, byte b2)
{
    chunk_write(chunk, (byte[]){ b1, b2 }, 2, parser.previous.line);
}

static void emit_return()
{
    emit_byte(OP_RETURN);
}

static byte make_constant(Value v)
{
    int constant = chunk_add_constant(chunk, v);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return (byte)constant;
}

static void emit_constant(Value v)
{
    emit_bytes2(OP_CONSTANT, make_constant(v));
}

static void end_compiler()
{
#ifdef DEBUG_PRINT_CODE
    if (!parser.had_error) {
        chunk_disassemble(chunk, "code");
    }
#endif
    emit_return();
}

static ParseRule *get_rule(TokenType type)
{
    return &parse_rules[type];
}

static void parse_precedence(Precedence precedence)
{
    advance();
    ParseFn prefix_rule_fn = get_rule(parser.previous.type)->prefix;
    if (prefix_rule_fn == NULL) {
        error("Expect expression.");
        return;
    }

    prefix_rule_fn();

    while (precedence <= get_rule(parser.current.type)->precedence) {
        advance();
        ParseFn infix_rule_fn = get_rule(parser.previous.type)->infix;
        infix_rule_fn();
    }
}

static void expression()
{
    parse_precedence(PREC_ASSIGNMENT);
}

static void grouping()
{
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number()
{
    double value = strtod(parser.previous.start, NULL);
    emit_constant(value);
}

static void unary()
{
    TokenType op_type = parser.previous.type;

    // Compile the operand.
    parse_precedence(PREC_UNARY);

    // Emit the operator instruction.
    switch (op_type) {
        case TOKEN_MINUS: emit_byte(OP_NEGATE); break;
        default: return; // Unreachable.
    }
}

static void binary()
{
    TokenType op_type = parser.previous.type;

    // Compile the right operand.
    ParseRule *rule = get_rule(op_type);
    parse_precedence((Precedence)(rule->precedence + 1));

    // Emit the operator instruction.
    switch (op_type) {
        case TOKEN_PLUS:  emit_byte(OP_ADD); break;
        case TOKEN_MINUS: emit_byte(OP_SUB); break;
        case TOKEN_STAR:  emit_byte(OP_MUL); break;
        case TOKEN_SLASH: emit_byte(OP_DIV); break;
        default:          return; // Unreachable.
    }
}

static bool compile(const char *source, Chunk *ch)
{
    chunk = ch;
    scanner_init(&scanner, source);
    parser.had_error = false;
    parser.panic_mode = false;

    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");
    end_compiler();
    return !parser.had_error;
}
