#include "compiler.h"

#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>

struct Parser
{
    MyMemory& mem;
    Scanner& scanner;
    Script& script;
    Token previous;
    Token current;
    bool hadError;
    bool panicMode;
};


static void errorAt(Parser& parser, const Token& token, const char* message)
{
    if(parser.panicMode)
        return;
    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token.line);
    if(token.type == TokenType::END_OF_FILE)
    {
        fprintf(stderr, "at end");
    }
    else if(token.type == TokenType::TOKEN_ERROR)
    {

    }
    else
    {
        fprintf(stderr, " at '%.*s'", token.len, token.start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;

}

static void errorAtCurrent(Parser& parser, const char* message)
{
    errorAt(parser, parser.current, message);
}
static void error(Parser& parser, const char* message)
{
    errorAt(parser, parser.previous, message);
}

static void emitByteCode(Parser& parser, Op code)
{
    addOpCode(parser.script, code, parser.previous.line);
}

static void advance(Parser& parser)
{
    parser.previous = parser.current;
    while(true)
    {
        parser.current = scanToken(parser.scanner);
        if(parser.current.type != TokenType::TOKEN_ERROR)
            break;

        errorAtCurrent(parser, (const char*)parser.current.start);
    }
}

static void consume(Parser& parser, TokenType type, const char* message)
{
    if (parser.current.type == type) {
        advance(parser);
        return;
    }

    errorAtCurrent(parser, message);
}

static void emitReturn(Parser& parser)
{
    emitByteCode(parser, OP_RETURN);
}

static void endCompiler(Parser& parser)
{
    emitReturn(parser);
}






static void expression(Parser& parser)
{
    
}


bool compile(MyMemory& mem, Script& script)
{
    const u8* src = mem.scriptFile.data();
    Scanner scanner = {
        .mem = mem,
        .src = src,
        .srcEnd = src + mem.scriptFile.size(),
        .startToken = src,
        .current = src,
        .line = 1,
        .hasErrors = false,
    };
    Parser parser = {
        .mem = mem,
        .scanner = scanner,
        .script = script,
        .hadError = false,
    };
    advance(parser);
    expression(parser);
    consume(parser, TokenType::END_OF_FILE, "Expect end of expression.");

    return !parser.hadError;
/*
    int line = -1;

    while(true)
    {
        Token token = scanToken(scanner);
        if (token.line != line)
        {
            printf("%4d ", token.line);
            line = token.line;
        }
        else
        {
            printf("   | ");
        }
        printf("%2d '%.*s'\n", (i32)token.type, token.len, token.start);

        if (token.type == TokenType::END_OF_FILE)
            break;
    }
    */

}
