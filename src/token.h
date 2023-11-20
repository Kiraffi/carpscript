#pragma once

#include "mytypes.h"



enum class TokenType: u8
{
    //Single character tokens
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR, COLON,

    // One or two character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESSER, LESSER_EQUAL,

    // Literals
    IDENTIFIER, LITERAL_BOOL, LITERAL_STRING, NUMBER, INTEGER,

    // Keywords
    I8, I16, I32, I64, U8, U16, U32, U64, F32, F64,
    STRING, BOOL,
    LET, MUT,
    STRUCT,
    AND, OR,
    ELSE, FN, FOR, IF, NIL, WHILE, NATCALL,
    PRINT, RETURN,
    TRUE, FALSE,

    TOKEN_ERROR,
    // End of file must be last
    END_OF_FILE,
};

static const char* TOKEN_NAMES[] = {
    //Single character tokens
    "LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACE", "RIGHT_BRACE",
    "COMMA", "DOT", "MINUS", "PLUS", "SEMICOLON", "SLASH", "STAR", "COLON",

    // One or two character tokens.
    "BANG", "BANG_EQUAL",
    "EQUAL", "EQUAL_EQUAL",
    "GREATER", "GREATER_EQUAL",
    "LESSER", "LESSER_EQUAL",

    // Literals
    "IDENTIFIER", "LITERAL_BOOL", "LITERAL_STRING", "NUMBER", "INTEGER",

    // Keywords
    "I8", "I16", "I32", "I64", "U8", "U16", "U32", "U64", "F32", "F64",
    "STRING", "BOOL",
    "LET", "MUT",
    "STRUCT",
    "AND", "OR",
    "ELSE", "FN", "FOR", "IF", "NIL", "WHILE", "NATCALL",
    "PRINT", "RETURN",
    "TRUE", "FALSE",

    "TOKEN_ERROR",
    // End of file must be last
    "END_OF_FILE",
};
static_assert(sizeof(TOKEN_NAMES) / sizeof(const char*) == (i32)(TokenType::END_OF_FILE) + 1, "Mismatched amounts");

struct Token
{
    const u8* start;
    i32 len;
    i32 line;
    TokenType type;
};
