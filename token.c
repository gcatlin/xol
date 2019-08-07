#pragma once

#include "common.h"

static const char *token_type_names[] = {
    [TOKEN_NONE]          = "TOKEN_NONE",

    [TOKEN_BANG]          = "TOKEN_BANG",
    [TOKEN_BANG_EQUAL]    = "TOKEN_BANG_EQUAL",
    [TOKEN_COMMA]         = "TOKEN_COMMA",
    [TOKEN_DOT]           = "TOKEN_DOT",
    [TOKEN_EQUAL]         = "TOKEN_EQUAL",
    [TOKEN_EQUAL_EQUAL]   = "TOKEN_EQUAL_EQUAL",
    [TOKEN_GREATER]       = "TOKEN_GREATER",
    [TOKEN_GREATER_EQUAL] = "TOKEN_GREATER_EQUAL",
    [TOKEN_LEFT_BRACE]    = "TOKEN_LEFT_BRACE",
    [TOKEN_LEFT_PAREN]    = "TOKEN_LEFT_PAREN",
    [TOKEN_LESS]          = "TOKEN_LESS",
    [TOKEN_LESS_EQUAL]    = "TOKEN_LESS_EQUAL",
    [TOKEN_MINUS]         = "TOKEN_MINUS",
    [TOKEN_PLUS]          = "TOKEN_PLUS",
    [TOKEN_RIGHT_BRACE]   = "TOKEN_RIGHT_BRACE",
    [TOKEN_RIGHT_PAREN]   = "TOKEN_RIGHT_PAREN",
    [TOKEN_SEMICOLON]     = "TOKEN_SEMICOLON",
    [TOKEN_SLASH]         = "TOKEN_SLASH",
    [TOKEN_STAR]          = "TOKEN_STAR",

    [TOKEN_IDENTIFIER]    = "TOKEN_IDENTIFIER",
    [TOKEN_STRING]        = "TOKEN_STRING",
    [TOKEN_NUMBER]        = "TOKEN_NUMBER",

    [TOKEN_AND]           = "TOKEN_AND",
    [TOKEN_CLASS]         = "TOKEN_CLASS",
    [TOKEN_ELSE]          = "TOKEN_ELSE",
    [TOKEN_FALSE]         = "TOKEN_FALSE",
    [TOKEN_FOR]           = "TOKEN_FOR",
    [TOKEN_FN]            = "TOKEN_FN",
    [TOKEN_IF]            = "TOKEN_IF",
    [TOKEN_NIL]           = "TOKEN_NIL",
    [TOKEN_OR]            = "TOKEN_OR",
    [TOKEN_PRINT]         = "TOKEN_PRINT",
    [TOKEN_RETURN]        = "TOKEN_RETURN",
    [TOKEN_SUPER]         = "TOKEN_SUPER",
    [TOKEN_THIS]          = "TOKEN_THIS",
    [TOKEN_TRUE]          = "TOKEN_TRUE",
    [TOKEN_VAR]           = "TOKEN_VAR",
    [TOKEN_WHILE]         = "TOKEN_WHILE",

    [TOKEN_COMMENT]       = "TOKEN_COMMENT",
    [TOKEN_ERROR]         = "TOKEN_ERROR",
    [TOKEN_EOF]           = "TOKEN_EOF",
};

static const char *token_type_name(TokenType t)
{
    return token_type_names[t];
}
