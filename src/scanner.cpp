#include "scanner.h"

#include "token.h"

#include <string.h> // strlen

struct Keyword
{
    const char* name;
    TokenType type;
    u32 len;
};

static constexpr Keyword keywords[]{
    Keyword{ "i8",     TokenType::I8, 2},
    Keyword{ "u8",     TokenType::U8, 2},
    Keyword{ "i16",    TokenType::I16, 3},
    Keyword{ "u16",    TokenType::U16, 3},
    Keyword{ "i32",    TokenType::I32, 3},
    Keyword{ "u32",    TokenType::U32, 3},
    Keyword{ "i64",    TokenType::I64, 3},
    Keyword{ "u64",    TokenType::U64, 3},
    Keyword{ "f32",    TokenType::F32, 3},
    Keyword{ "f64",    TokenType::F64, 3},
    Keyword{ "let",    TokenType::LET, 3},
    Keyword{ "bool",   TokenType::BOOL, 4},
    Keyword{ "mut",    TokenType::MUT, 3},
    Keyword{ "struct", TokenType::STRUCT, 6},
    Keyword{ "string", TokenType::STRING, 6},

    Keyword{ "and", TokenType::AND, 3 },
    Keyword{ "&&", TokenType::AND, 2 },
    Keyword{ "else", TokenType::ELSE, 4 },
    Keyword{ "false", TokenType::FALSE, 5 },
    Keyword{ "true", TokenType::TRUE, 4 },
    Keyword{ "fn", TokenType::FN, 2 },
    Keyword{ "if", TokenType::IF, 2 },
    Keyword{ "nil", TokenType::NIL, 3 },
    Keyword{ "or", TokenType::OR, 2 },
    Keyword{ "||", TokenType::OR, 2 },
    Keyword{ "print", TokenType::PRINT, 5 },
    Keyword{ "return", TokenType::RETURN, 6 },
    Keyword{ "while", TokenType::WHILE, 5 },
};

static i32 getScannerCurrLenToStartToken(const Scanner& scanner)
{
    return i32(intptr_t(scanner.current) - intptr_t(scanner.startToken));
}

static bool isAtAtEnd(const Scanner& scanner)
{
    return scanner.current >= scanner.srcEnd;
}

static u8 peek(const Scanner& scanner, i32 amount)
{
    if (scanner.current + amount >= scanner.srcEnd)
    {
        return '\0';
    }
    if (scanner.current + amount < scanner.src)
    {
        return '\0';
    }
    u8 c = *(scanner.current + amount);
    return c;
}

static u8 peek(const Scanner& scanner)
{
    if (scanner.current >= scanner.srcEnd)
    {
        return '\0';
    }

    return peek(scanner, 0);
}

static u8 advance(Scanner& scanner)
{
    if (scanner.current >= scanner.srcEnd)
    {
        return '\0';
    }
    u8 c = peek(scanner);
    scanner.current++;
    return c;
}


static bool matchChar(Scanner& scanner, char c)
{
    if (scanner.current >= scanner.srcEnd)
    {
        return false;
    }
    if ((char)peek(scanner) != c)
    {
        return false;
    }
    advance(scanner);
    return true;
}

static void skipWhiteSpace(Scanner& scanner)
{
    while(true)
    {
        char c = (char)peek(scanner);
        switch(c)
        {
            case '\t':
            case '\r':
            case ' ':
                advance(scanner);
                break;
            case '\n':
                scanner.line++;
                advance(scanner);
                break;
            default:
                return;
        }
    }
}


static Token makeToken(const Scanner& scanner, TokenType type)
{
    Token token {
        .start = scanner.startToken,
        .len = getScannerCurrLenToStartToken(scanner),
        .line = scanner.line,
        .type = type,
    };
    return token;
}

static Token errorToken(const Scanner& scanner, const char* message)
{
    Token token {
        .start = (u8*)message,
        .len = (i32)strlen(message),
        .line = scanner.line,
        .type = TokenType::TOKEN_ERROR,
    };
    return token;
}

static Token handleNumberString(Scanner& scanner)
{
    while (isdigit(peek(scanner)))
    {
        advance(scanner);
    }

    if (peek(scanner) == '.' && isdigit(peek(scanner, 1)))
    {
        advance(scanner);
        while (isdigit(peek(scanner)))
        {
            advance(scanner);
        }
    }
    else
    {
        return makeToken(scanner, TokenType::INTEGER);
    }
    return makeToken(scanner, TokenType::NUMBER);
}

static Token handleIdentifier(Scanner& scanner)
{
    while (isAlphaNumUnderscore((char)peek(scanner)))
    {
        advance(scanner);
    }
    i32 sz = getScannerCurrLenToStartToken(scanner);
    const char* identifier = (const char*)&scanner.startToken;
    for (const Keyword& word : keywords)
    {
        if (sz == word.len && strncmp(identifier, word.name, word.len) == 0)
        {
            return makeToken(scanner, word.type);
        }
    }

    return makeToken(scanner, TokenType::IDENTIFIER);
}


static Token handleStringLiteral(Scanner& scanner)
{
    while (peek(scanner) != '"' && !isAtAtEnd(scanner))
    {
        if (peek(scanner) == '\n')
        {
            scanner.line++;
        }
        advance(scanner);
    }

    if (isAtAtEnd(scanner))
    {
        return errorToken(scanner, "Unterminated string!");
    }

    advance(scanner);
    return makeToken(scanner, TokenType::LITERAL_STRING);
}


Token scanToken(Scanner& scanner)
{
    skipWhiteSpace(scanner);
    scanner.startToken = scanner.current;

    if(isAtAtEnd(scanner))
        return makeToken(scanner, TokenType::END_OF_FILE);

    char c = (char)advance(scanner);
    switch (c)
    {
        case '(': return makeToken(scanner, TokenType::LEFT_PAREN); break;
        case ')': return makeToken(scanner, TokenType::RIGHT_PAREN); break;
        case '{': return makeToken(scanner, TokenType::LEFT_BRACE); break;
        case '}': return makeToken(scanner, TokenType::RIGHT_BRACE); break;
        case ',': return makeToken(scanner, TokenType::COMMA); break;
        case '.': return makeToken(scanner, TokenType::DOT); break;
        case '-': return makeToken(scanner, TokenType::MINUS); break;
        case '+': return makeToken(scanner, TokenType::PLUS); break;
        case ';': return makeToken(scanner, TokenType::SEMICOLON); break;
        case '*': return makeToken(scanner, TokenType::STAR); break;
        case '\0': return makeToken(scanner, TokenType::END_OF_FILE); break;
            // Double char checks
        case '!': return makeToken(scanner, matchChar(scanner, '=') ? TokenType::BANG_EQUAL : TokenType::BANG); break;
        case '=': return makeToken(scanner, matchChar(scanner, '=') ? TokenType::EQUAL_EQUAL : TokenType::EQUAL); break;
        case '<': return makeToken(scanner, matchChar(scanner, '=') ? TokenType::LESSER_EQUAL : TokenType::LESSER); break;
        case '>': return makeToken(scanner, matchChar(scanner, '=') ? TokenType::GREATER_EQUAL : TokenType::GREATER); break;

            // comments //
        case '/':
            if (matchChar(scanner, '/'))
            {
                while (peek(scanner) != '\n' && !isAtAtEnd(scanner))
                {
                    advance(scanner);
                }
            }
            else
            {
                return makeToken(scanner, TokenType::SLASH);
            }
            break;

            // String literal
        case '"': return handleStringLiteral(scanner); break;

        default:
        {
            bool handled = false;
            if (isdigit(c))
            {
                return handleNumberString(scanner);
            }
            else if (isAlphaUnderscore(c))
            {
                return handleIdentifier(scanner);
            }
            else if(c == '|' && matchChar(scanner, '|'))
            {
                return makeToken(scanner, TokenType::OR);
            }
            else if(c == '&' && matchChar(scanner, '&'))
            {
                return makeToken(scanner, TokenType::AND);
            }
            else
            {
                return errorToken(scanner, "Unexpected error on scanner.");
            }
            break;
        }
    }

    return errorToken(scanner, "Unexpected error on scanner, reached the end of scan token.");

}

