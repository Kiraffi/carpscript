#pragma once

#include "mytypes.h"



enum class TokenType: u8
{
    //Single character tokens
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

    // One or two character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESSER, LESSER_EQUAL,

    // Literals
    IDENTIFIER, STRING, INTEGER, NUMBER,

    // Keywords
    CLASS, SUPER,
    AND, OR,
    ELSE, FUNC, FOR, IF, NIL, THIS, WHILE,
    PRINT, RETURN,
    TRUE, FALSE, VAR,


    // End of file must be last
    END_OF_FILE,
};

static const char* TOKEN_NAMES[] = {
    //Single character tokens
    "LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACE", "RIGHT_BRACE",
    "COMMA", "DOT", "MINUS", "PLUS", "SEMICOLON", "SLASH", "STAR",

    // One or two character tokens.
    "BANG", "BANG_EQUAL",
    "EQUAL", "EQUAL_EQUAL",
    "GREATER", "GREATER_EQUAL",
    "LESSER", "LESSER_EQUAL",

    // Literals
    "IDENTIFIER", "STRING", "INTEGER", "NUMBER",

    // Keywords
    "CLASS", "SUPER",
    "AND", "OR",
    "ELSE", "FUNC", "FOR", "IF", "NIL", "THIS", "WHILE",
    "PRINT", "RETURN",
    "TRUE", "FALSE", "VAR",


    // End of file must be last
    "END_OF_FILE",
};
static_assert(sizeof(TOKEN_NAMES) / sizeof(const char*) == (i32)(TokenType::END_OF_FILE) + 1);

struct Token
{
    const char* start;
    u32 len;
    TokenType type;
};
