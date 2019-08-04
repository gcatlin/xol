#pragma once

#include "common.h"
#include "token.c"

static bool is_alpha(const char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

typedef struct {
    const char *start;   // token
    const char *current; // cursor
    int         line;
} Scanner;

static void scanner_init(Scanner *s, const char *source)
{
    s->start = source;
    s->current = source;
    s->line = 1;
}

static char scanner_advance(Scanner *s)
{
    s->current++;
    return s->current[-1];
}

static bool inline scanner_eof(Scanner *s)
{
    return *s->current == '\0';
}

static bool scanner_match(Scanner *s, char expected)
{
    if (scanner_eof(s) || *s->current != expected) {
        return false;
    }
    s->current++;
    return true;
}

static char scanner_peek(Scanner *s)
{
    return *s->current;
}

static char scanner_peek_next(Scanner *s)
{
    if (scanner_eof(s)) {
        return '\0';
    }
    return s->current[1];
}

static void scanner_skip_whitespace(Scanner *s)
{
    for (;;) {
        char c = *s->current;
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                scanner_advance(s);
                break;
            case '\n':
                s->line++;
                scanner_advance(s);
                break;
            case '/':
                if (scanner_peek_next(s) == '/') {
                    while (scanner_peek(s) != '\n' && !scanner_eof(s)) {
                        scanner_advance(s);
                    }
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static Token scanner_make_token(Scanner *s, TokenType t)
{
    return (Token){
        .type   = t,
        .start  = s->start,
        .length = (int)(s->current - s->start),
        .line   = s->line,
    };
}

static Token scanner_error_token(Scanner *s, const char *message)
{
    return (Token){
        .type   = TOKEN_ERROR,
        .start  = message,
        .length = (int)strlen(message),
        .line   = s->line,
    };
}

static TokenType scanner_check_keyword(Scanner *s, int start, int length, const char *rest, TokenType type)
{
    if ((s->current - s->start == start + length) &&
            memcmp(s->start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType scanner_identifier_type(Scanner *s)
{
    switch (s->start[0]) {
        case 'a': return scanner_check_keyword(s, 1, 2, "nd",    TOKEN_AND);
        case 'c': return scanner_check_keyword(s, 1, 4, "lass",  TOKEN_CLASS);
        case 'e': return scanner_check_keyword(s, 1, 3, "lse",   TOKEN_ELSE);
        case 'f':
            if (s->current - s->start > 1) {
              switch (s->start[1]) {
                  case 'a': return scanner_check_keyword(s, 2, 3, "lse", TOKEN_FALSE);
                  case 'o': return scanner_check_keyword(s, 2, 1, "r",   TOKEN_FOR);
                  case 'u': return scanner_check_keyword(s, 2, 1, "n",   TOKEN_FN);
              }
            }
            break;
        case 'i': return scanner_check_keyword(s, 1, 1, "f",     TOKEN_IF);
        case 'n': return scanner_check_keyword(s, 1, 2, "il",    TOKEN_NIL);
        case 'o': return scanner_check_keyword(s, 1, 1, "r",     TOKEN_OR);
        case 'p': return scanner_check_keyword(s, 1, 4, "rint",  TOKEN_PRINT);
        case 'r': return scanner_check_keyword(s, 1, 5, "eturn", TOKEN_RETURN);
        case 's': return scanner_check_keyword(s, 1, 4, "uper",  TOKEN_SUPER);
        case 't':
            if (s->current - s->start > 1) {
                switch (s->start[1]) {
                    case 'h': return scanner_check_keyword(s, 2, 2, "is", TOKEN_THIS);
                    case 'r': return scanner_check_keyword(s, 2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
        case 'v': return scanner_check_keyword(s, 1, 2, "ar",    TOKEN_VAR);
        case 'w': return scanner_check_keyword(s, 1, 4, "hile",  TOKEN_WHILE);
    }

    return TOKEN_IDENTIFIER;
}

static Token scanner_identifier_token(Scanner *s)
{
    while (is_alpha(scanner_peek(s)) || is_digit(scanner_peek(s))) {
        scanner_advance(s);
    }

    return scanner_make_token(s, scanner_identifier_type(s));
}

static Token scanner_number_token(Scanner *s)
{
    while (is_digit(scanner_peek(s))) {
        scanner_advance(s);
    }

    // Look for a fractional part.
    if (scanner_peek(s) == '.' && is_digit(scanner_peek_next(s))) {
        // Consume the "."
        scanner_advance(s);

        while (is_digit(scanner_peek(s))) {
            scanner_advance(s);
        }
    }

    return scanner_make_token(s, TOKEN_NUMBER);
}

static Token scanner_string_token(Scanner *s)
{
    while (scanner_peek(s) != '"' && !scanner_eof(s)) {
        if (scanner_peek(s) == '\n') {
            s->line++;
        }
        scanner_advance(s);
    }

    if (scanner_eof(s)) {
        return scanner_error_token(s, "Unterminated string.");
    }

    // close "
    scanner_advance(s);
    return scanner_make_token(s, TOKEN_STRING);
}

static Token scanner_scan_token(Scanner *s)
{
    scanner_skip_whitespace(s);
    s->start = s->current;

    if (scanner_eof(s)) {
        return scanner_make_token(s, TOKEN_EOF);
    }

    char c = scanner_advance(s);
    if (is_alpha(c)) return scanner_identifier_token(s);
    if (is_digit(c)) return scanner_number_token(s);

    switch (c) {
        case '(': return scanner_make_token(s, TOKEN_LEFT_PAREN);
        case ')': return scanner_make_token(s, TOKEN_RIGHT_PAREN);
        case '{': return scanner_make_token(s, TOKEN_LEFT_BRACE);
        case '}': return scanner_make_token(s, TOKEN_RIGHT_BRACE);
        case ';': return scanner_make_token(s, TOKEN_SEMICOLON);
        case ',': return scanner_make_token(s, TOKEN_COMMA);
        case '.': return scanner_make_token(s, TOKEN_DOT);
        case '-': return scanner_make_token(s, TOKEN_MINUS);
        case '+': return scanner_make_token(s, TOKEN_PLUS);
        case '/': return scanner_make_token(s, TOKEN_SLASH);
        case '*': return scanner_make_token(s, TOKEN_STAR);
        case '!': return scanner_make_token(s, scanner_match(s, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=': return scanner_make_token(s, scanner_match(s, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<': return scanner_make_token(s, scanner_match(s, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>': return scanner_make_token(s, scanner_match(s, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"': return scanner_string_token(s);
    }

    return scanner_error_token(s, "Unexpected character.");
}
