#include "compiler.h"

#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>

#if DEBUG_PRINT_CODE
#include "debug.h"
#endif

struct Parser;

using ParseFn = void(*)(Parser& parser);

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


enum Precedence
{
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY,
};

struct ParseRule
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
};



static void expression(Parser& parser);
static void parsePrecedence(Parser& parser, Precedence precedence);

static void grouping(Parser& parser);
static void unary(Parser& parser);
static void binary(Parser& parser);
static void number(Parser& parser);

// C99 feature, not c++...
//ParseRule rules[] =
//{
//    [LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
//    [RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
//    [LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE},
//    [RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
//    [COMMA]         = {NULL,     NULL,   PREC_NONE},
//    [DOT]           = {NULL,     NULL,   PREC_NONE},
//    [MINUS]         = {unary,    binary, PREC_TERM},
//    [PLUS]          = {NULL,     binary, PREC_TERM},
//    [SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
//    [SLASH]         = {NULL,     binary, PREC_FACTOR},
//    [STAR]          = {NULL,     binary, PREC_FACTOR},
//    [BANG]          = {NULL,     NULL,   PREC_NONE},
//    [BANG_EQUAL]    = {NULL,     NULL,   PREC_NONE},
//    [EQUAL]         = {NULL,     NULL,   PREC_NONE},
//    [EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
//    [GREATER]       = {NULL,     NULL,   PREC_NONE},
//    [GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
//    [LESSER]        = {NULL,     NULL,   PREC_NONE},
//    [LESSER_EQUAL]  = {NULL,     NULL,   PREC_NONE},
//    [IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
//    [STRING]        = {NULL,     NULL,   PREC_NONE},
//    [NUMBER]        = {number,   NULL,   PREC_NONE},
//    [AND]           = {NULL,     NULL,   PREC_NONE},
//    [STRUCT]        = {NULL,     NULL,   PREC_NONE},
//    [ELSE]          = {NULL,     NULL,   PREC_NONE},
//    [FALSE]         = {NULL,     NULL,   PREC_NONE},
//    [FOR]           = {NULL,     NULL,   PREC_NONE},
//    [FN]            = {NULL,     NULL,   PREC_NONE},
//    [IF]            = {NULL,     NULL,   PREC_NONE},
//    [NIL]           = {NULL,     NULL,   PREC_NONE},
//    [OR]            = {NULL,     NULL,   PREC_NONE},
//    [PRINT]         = {NULL,     NULL,   PREC_NONE},
//    [RETURN]        = {NULL,     NULL,   PREC_NONE},
//    [TRUE]          = {NULL,     NULL,   PREC_NONE},
//    [WHILE]         = {NULL,     NULL,   PREC_NONE},
//    [TOKEN_ERROR]   = {NULL,     NULL,   PREC_NONE},
//    [END_OF_FILE]   = {NULL,     NULL,   PREC_NONE},
//};




static ParseRule getRule(TokenType type)
{


    switch(type)
    {
        case TokenType::LEFT_PAREN:       return {grouping, NULL,   PREC_NONE};   break;
        case TokenType::RIGHT_PAREN:      return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::LEFT_BRACE:       return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::RIGHT_BRACE:      return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::COMMA:            return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::DOT:              return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::MINUS:            return {unary,    binary, PREC_TERM};   break;
        case TokenType::PLUS:             return {NULL,     binary, PREC_TERM};   break;
        case TokenType::SEMICOLON:        return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::SLASH:            return {NULL,     binary, PREC_FACTOR}; break;
        case TokenType::STAR:             return {NULL,     binary, PREC_FACTOR}; break;
        case TokenType::BANG:             return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::BANG_EQUAL:       return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::EQUAL:            return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::EQUAL_EQUAL:      return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::GREATER:          return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::GREATER_EQUAL:    return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::LESSER:           return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::LESSER_EQUAL:     return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::IDENTIFIER:       return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::STRING:           return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::NUMBER:           return {number,   NULL,   PREC_NONE};   break;
        case TokenType::INTEGER:          return {number,   NULL,   PREC_NONE};   break;
        case TokenType::AND:              return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::STRUCT:           return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::ELSE:             return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::FALSE:            return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::FOR:              return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::FN:               return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::IF:               return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::NIL:              return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::OR:               return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::PRINT:            return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::RETURN:           return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::TRUE:             return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::WHILE:            return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::TOKEN_ERROR:      return {NULL,     NULL,   PREC_NONE};   break;
        case TokenType::END_OF_FILE:      return {NULL,     NULL,   PREC_NONE};   break;
        default:
            return {NULL, NULL, PREC_NONE};
    }
}

static void emitBytes(Parser& parser, OpCodeType first, OpCodeType second)
{
    addOpCode(parser.script, (Op)first, parser.previous.line);
    addOpCode(parser.script, (Op)second, parser.current.line);
}
static void emitByteCode(Parser& parser, Op code)
{
    addOpCode(parser.script, code, parser.previous.line);
}


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
#if DEBUG_PRINT_CODE
    if(!parser.hadError)
    {
        disassembleCode(parser.script, "code");
    }
#endif
    emitReturn(parser);
}


static void number(Parser& parser)
{
    if(parser.previous.type == TokenType::INTEGER)
    {
        char* end;
        i32 value = (i32)strtol((const char*)parser.previous.start, &end, 10);
        addOpCode(parser.script, OP_CONSTANT_I32, parser.previous.line);
        addConstant(parser.script, 0, value, parser.previous.line);

    }
    else
    {
        char* end;
        f32 value = strtof((const char*)parser.previous.start, &end);
        addOpCode(parser.script, OP_CONSTANT_F32, parser.previous.line);
        addConstant(parser.script, 0, value, parser.previous.line);

    }
}

static void grouping(Parser& parser)
{
    expression(parser);
    consume(parser, TokenType::RIGHT_PAREN, "Expect ')' after expression.");
}

static void unary(Parser& parser)
{
    TokenType operatorType = parser.previous.type;

    parsePrecedence(parser, Precedence::PREC_UNARY);

    switch(operatorType)
    {
        case TokenType::MINUS: emitByteCode(parser, OP_NEGATE); break;
        default: return;
    }
};

static void binary(Parser& parser)
{
    TokenType operatorType = parser.previous.type;
    const ParseRule& rule = getRule(operatorType);
    parsePrecedence(parser, Precedence(rule.precedence + 1));

    switch(operatorType)
    {
        case TokenType::PLUS:  emitByteCode(parser, OP_ADD); break;
        case TokenType::MINUS: emitByteCode(parser, OP_SUB); break;
        case TokenType::STAR:  emitByteCode(parser, OP_MUL); break;
        case TokenType::SLASH: emitByteCode(parser, OP_DIV); break;
        default: return;
    }
}


static void parsePrecedence(Parser& parser, Precedence precedence)
{
    advance(parser);
    ParseFn prefixRule = getRule(parser.previous.type).prefix;
    if(prefixRule == nullptr)
    {
        error(parser, "Expect expression.");
        return;
    }
    prefixRule(parser);

    while(precedence <= getRule(parser.current.type).precedence)
    {
        advance(parser);
        getRule(parser.previous.type).infix(parser);
    }
}

static void expression(Parser& parser)
{
    parsePrecedence(parser, Precedence::PREC_ASSIGNMENT);

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
