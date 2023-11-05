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
    Token next;
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
static void literal(Parser& parser);
static void string(Parser& parser);
static void variable(Parser& parser);

static void statement(Parser& parser);
static void declaration(Parser& parser);
static void printStatement(Parser& parser);

static void advance(Parser& parser);


static bool check(const Parser& parser, TokenType type)
{
    return parser.current.type == type;
}

static bool check2(const Parser &parser, TokenType type)
{
    return parser.next.type == type;
}

static bool match(Parser& parser, TokenType type)
{
    if(!check(parser, type))
        return false;
    advance(parser);
    return true;
}

i32 addOpCode(Parser& parser, Op op)
{
    return addOpCode(parser.script, op, parser.previous.line);
}

static ParseRule getRule(TokenType type)
{


    switch(type)
    {
        case TokenType::LEFT_PAREN:       return {grouping, NULL,   PREC_NONE};         break;
        case TokenType::RIGHT_PAREN:      return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::LEFT_BRACE:       return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::RIGHT_BRACE:      return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::COMMA:            return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::DOT:              return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::MINUS:            return {unary,    binary, PREC_TERM};         break;
        case TokenType::PLUS:             return {NULL,     binary, PREC_TERM};         break;
        case TokenType::SEMICOLON:        return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::SLASH:            return {NULL,     binary, PREC_FACTOR};       break;
        case TokenType::STAR:             return {NULL,     binary, PREC_FACTOR};       break;
        case TokenType::BANG:             return {unary,    NULL,   PREC_NONE};         break;
        case TokenType::BANG_EQUAL:       return {NULL,     binary, PREC_EQUALITY};     break;
        case TokenType::EQUAL:            return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::EQUAL_EQUAL:      return {NULL,     binary, PREC_COMPARISON};   break;
        case TokenType::GREATER:          return {NULL,     binary, PREC_COMPARISON};   break;
        case TokenType::GREATER_EQUAL:    return {NULL,     binary, PREC_COMPARISON};   break;
        case TokenType::LESSER:           return {NULL,     binary, PREC_COMPARISON};   break;
        case TokenType::LESSER_EQUAL:     return {NULL,     binary, PREC_COMPARISON};   break;
        case TokenType::IDENTIFIER:       return {variable, NULL,   PREC_NONE};         break;
        case TokenType::LITERAL_STRING:   return {string,   NULL,   PREC_NONE};         break;
        case TokenType::NUMBER:           return {number,   NULL,   PREC_NONE};         break;
        case TokenType::INTEGER:          return {number,   NULL,   PREC_NONE};         break;
        case TokenType::AND:              return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::STRUCT:           return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::ELSE:             return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::FALSE:            return {literal,  NULL,   PREC_NONE};         break;
        case TokenType::FOR:              return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::FN:               return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::IF:               return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::NIL:              return {literal,  NULL,   PREC_NONE};         break;
        case TokenType::OR:               return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::PRINT:            return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::RETURN:           return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::TRUE:             return {literal,  NULL,   PREC_NONE};         break;
        case TokenType::WHILE:            return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::TOKEN_ERROR:      return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::END_OF_FILE:      return {NULL,     NULL,   PREC_NONE};         break;
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
    parser.current = parser.next;

    while(true)
    {
        parser.next = scanToken(parser.scanner);
        if(parser.next.type != TokenType::TOKEN_ERROR)
            break;

        errorAtCurrent(parser, (const char*)parser.next.start);
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
        emitByteCode(parser, OP_CONSTANT_I32);
        addConstant(parser.script, value, parser.previous.line);

    }
    else
    {
        char* end;
        f32 value = strtof((const char*)parser.previous.start, &end);
        emitByteCode(parser, OP_CONSTANT_F32);
        addConstant(parser.script, value, parser.previous.line);

    }
}

static void literal(Parser& parser)
{
    switch(parser.previous.type)
    {
        case TokenType::FALSE: emitByteCode(parser, OP_CONSTANT_BOOL);  addConstant(parser.script, false, parser.previous.line); break;
        case TokenType::TRUE:  emitByteCode(parser, OP_CONSTANT_BOOL);  addConstant(parser.script, true, parser.previous.line); break;
        case TokenType::NIL:   emitByteCode(parser, OP_NIL); break;
        default: return;
    }
}

static i32 namedVariable(Parser& parser, const Token& token, i32& outStructIndex)
{
    i32 structIndex = parser.script.structIndex;
    outStructIndex = structIndex;
    std::string searchString = getStringFromTokenName(token);
    while(structIndex != -1)
    {
        const StructStack& s = parser.script.structStacks[structIndex];
        for(i32 index = 0; index < s.structSymbolNameIndices.size();  ++index)
        {
            i32 realIndex = index;
            if(parser.script.allSymbolNames[s.structSymbolNameIndices[realIndex]] == searchString)
            {
                return index;
            }
        }
        structIndex = s.parentStructIndex;
        outStructIndex = structIndex;
    }
    std::string errorStr = "No variable: ";
    errorStr += searchString;
    errorStr += " defined for getting.";
    errorAt(parser, token, errorStr.c_str());
    return -1;
}

static void variable(Parser& parser)
{
    i32 structIndex = -1;
    Token previous = parser.previous;
    i32 index = namedVariable(parser, previous, structIndex);


    if(structIndex >= 0 && structIndex < (i32)parser.script.structStacks.size()
       && index >= 0 && index < (i32) parser.script.structStacks[structIndex].structSymbolNameIndices.size())
    {
        bool popping = structIndex != parser.script.structIndex;
        if(popping)
        {
            emitByteCode(parser, OP_STACK_SET);
            emitByteCode(parser, Op(structIndex));
        }

        emitByteCode(parser, OP_GET_GLOBAL);
        emitByteCode(parser, Op(index));
        if(popping)
        {
            emitByteCode(parser, OP_STACK_SET);
            emitByteCode(parser, Op(parser.script.structIndex));

        }
    }
    else
    {
        errorAt(parser, previous, "failed to find proper parsing thingy for getter");
    }
}

static void string(Parser& parser)
{
    std::string s = std::string((const char*)parser.previous.start + 1, parser.previous.len - 2);

    emitByteCode(parser, OP_CONSTANT_STRING);
    addConstantString(parser.script, s, parser.previous.line);
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
        case TokenType::BANG:  emitByteCode(parser, OP_NOT); break;
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
        case TokenType::BANG_EQUAL:     emitBytes(parser, OP_EQUAL, OP_NOT); break;
        case TokenType::EQUAL_EQUAL:    emitByteCode(parser,OP_EQUAL); break;
        case TokenType::GREATER:        emitByteCode(parser,OP_GREATER); break;
        case TokenType::GREATER_EQUAL:  emitBytes(parser, OP_LESSER, OP_NOT); break;
        case TokenType::LESSER:         emitByteCode(parser, OP_LESSER); break;
        case TokenType::LESSER_EQUAL:   emitBytes(parser, OP_GREATER, OP_NOT); break;

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
    if(check(parser, TokenType::IDENTIFIER) && check2(parser, TokenType::EQUAL))
    {
        advance(parser);
        const Token previousToken = parser.previous;
        advance(parser);

        expression(parser);

        i32 structIndex = -1;
        i32 index = namedVariable(parser, previousToken, structIndex);

        if(structIndex >= 0 && structIndex < (i32)parser.script.structStacks.size()
            && index >= 0 && index < (i32) parser.script.structStacks[structIndex].structSymbolNameIndices.size())
        {
            bool popping = structIndex != parser.script.structIndex;
            if(popping)
            {
                emitByteCode(parser, OP_STACK_SET);
                emitByteCode(parser, Op(structIndex));

            }
            emitByteCode(parser, OP_SET_GLOBAL);
            emitByteCode(parser, Op(index));
            if(popping)
            {
                emitByteCode(parser, OP_STACK_SET);
                emitByteCode(parser, Op(parser.script.structIndex));

            }
        }
        else
        {
            errorAt(parser, previousToken, "failed to find proper parsing thingy for assignment");
        }
    }

    else
    {
        parsePrecedence(parser, Precedence::PREC_ASSIGNMENT);
    }
}

static void expressionStatement(Parser& parser)
{
    expression(parser);
    consume(parser, TokenType::SEMICOLON, "Expected ';' after expression.");
    emitByteCode(parser, OP_POP);
}


static void beginScope(Parser &parser)
{
    i32 index = addStruct(parser.script, "block", parser.script.structIndex);

    emitByteCode(parser, OP_STACK_SET);
    emitByteCode(parser, Op(index));


    // currentScope->scopeDepth++;
}

static void endScope(Parser &parser)
{
    StructStack &current = parser.script.structStacks[parser.script.structIndex];
    i32 parentStructIndex = current.parentStructIndex;
    assert(parentStructIndex >= 0 && parentStructIndex < parser.script.structStacks.size());

    emitByteCode(parser, OP_STACK_POP);

    parser.script.structIndex = parentStructIndex;
}

static void block(Parser& parser)
{
    while(!check(parser, TokenType::RIGHT_BRACE) && !check(parser, TokenType::END_OF_FILE))
    {
        declaration(parser);
    }

    consume(parser, TokenType::RIGHT_BRACE, "Expect '}' after block.");
}

static void statement(Parser& parser)
{
    if(match(parser, TokenType::PRINT))
    {
        printStatement(parser);
    }
    else if(match(parser, TokenType::LEFT_BRACE))
    {
        beginScope(parser);
        block(parser);
        endScope(parser);
    }
    else
    {
        expressionStatement(parser);
    }
}

static void printStatement(Parser& parser)
{
    expression(parser);
    consume(parser, TokenType::SEMICOLON, "Expect ';' after value.");
    emitByteCode(parser, OP_PRINT);
}

static void synchronize(Parser& parser)
{
    while(parser.current.type != TokenType::END_OF_FILE)
    {
        if(parser.previous.type == TokenType::SEMICOLON)
            return;

        switch(parser.current.type)
        {
            case TokenType::STRUCT:
            case TokenType::FN:
            case TokenType::LET:
            case TokenType::IF:
            case TokenType::WHILE:
            case TokenType::PRINT:
            case TokenType::RETURN:
                return;
            default:
                break;
        }
        advance(parser);
    }
}

static i32 identifierConstant(Parser& parser, const Token& token)
{
    std::string str = getStringFromTokenName(token);

    StructStack& sta = parser.script.structStacks[parser.script.structIndex];
    bool found = false;
    for(i32 i = 0; i < sta.structSymbolNameIndices.size(); ++i)
    {
        i32 realIndex = i;
        if(realIndex < parser.script.allSymbolNames.size() && parser.script.allSymbolNames[realIndex] == str)
        {
            found = true;
            break;
        }
    }
    i32 newIndex = -1;
    if(found)
    {
        std::string ss = "Variable ";
        ss += str;
        ss += " has already been defined!";
        errorAt(parser, token, ss.c_str());
    }
    else
    {
        i32 symbolIndex = addSymbolName(parser.script, str.c_str());
        newIndex = (i32)sta.structSymbolNameIndices.size();
        sta.structSymbolNameIndices.push_back(symbolIndex);
        sta.structValueTypes.push_back({});
        sta.structValueArray.push_back({});
    }
    return newIndex;
}

static i32 parseVariable(Parser& parser, const char* errorMessage)
{
    consume(parser, TokenType::IDENTIFIER, errorMessage);
    return identifierConstant(parser, parser.previous);
}

static void defineVariable(Parser& parser, i32 index)
{
    emitByteCode(parser, OP_DEFINE_GLOBAL);
    emitByteCode(parser, Op(index));
}

static void letDeclaration(Parser& parser)
{
    consume(parser, TokenType::IDENTIFIER, "Expect variable name.");
    Token previous = parser.previous;
    consume(parser, TokenType::EQUAL, "Expect '=' with variable declaration.");
    expression(parser);

    consume(parser, TokenType::SEMICOLON, "Expect ';' after variable declaration.");

    i32 global = identifierConstant(parser, previous);

    defineVariable(parser, global);
}

static void declaration(Parser& parser)
{
    if(match(parser, TokenType::LET))
    {
        letDeclaration(parser);
    }
    else
    {
        statement(parser);
    }
    if(parser.panicMode)
    {
        synchronize(parser);
    }
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
    advance(parser);
    //expression(parser);
    //consume(parser, TokenType::END_OF_FILE, "Expect end of expression.");

    while(!match(parser, TokenType::END_OF_FILE))
    {
        declaration(parser);
    }

    endCompiler(parser);
    return !parser.hadError;
}
