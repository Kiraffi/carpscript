#include "compiler.h"

#include "scanner.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // memcmp

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
    Token previousPrevious;
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

static void consume(Parser& parser, TokenType type, const char* message);


static void expression(Parser& parser);
static void parsePrecedence(Parser& parser, Precedence precedence);

static void grouping(Parser& parser);
static void unary(Parser& parser);
static void binary(Parser& parser);
static void number(Parser& parser);
static void literal(Parser& parser);
static void string(Parser& parser);
static void variable(Parser& parser);
static void andFn(Parser& parser);
static void orFn(Parser& parser);
static void fnCall(Parser& parser);

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
        case TokenType::LEFT_PAREN:       return {grouping, fnCall, PREC_CALL};         break;
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
        case TokenType::AND:              return {NULL,     andFn,  PREC_AND};          break;
        case TokenType::STRUCT:           return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::ELSE:             return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::FALSE:            return {literal,  NULL,   PREC_NONE};         break;
        case TokenType::FOR:              return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::FN:               return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::IF:               return {NULL,     NULL,   PREC_NONE};         break;
        case TokenType::NIL:              return {literal,  NULL,   PREC_NONE};         break;
        case TokenType::OR:               return {NULL,     orFn,   PREC_OR};           break;
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
    parser.previousPrevious = parser.previous;
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
    emitByteCode(parser, OP_NIL);
    emitByteCode(parser, OP_RETURN);
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

static i32 emitJump(Parser& parser, Op op)
{
    emitByteCode(parser, op);
    emitByteCode(parser, Op(0xffff));
    emitByteCode(parser, Op(0xffff));
    return parser.script.byteCode.size() - 2;
}
static void patchJump(Parser& parser, i32 offset)
{
    i64 jump = parser.script.byteCode.size() - offset - 2;
    if(offset > INT32_MAX || offset < INT32_MIN)
    {
        error(parser, "Too much code to jump over.");
    }

    parser.script.byteCode[offset + 0] = (jump >> 0) & 0xffff;
    parser.script.byteCode[offset + 1] = (jump >> 16) & 0xffff;
}

static void patchJumpAbsolute(Parser& parser, i32 codeAddressToPatch, i32 absoluteJumpAddress)
{
    parser.script.byteCode[codeAddressToPatch + 0] = (absoluteJumpAddress >> 0) & 0xffff;
    parser.script.byteCode[codeAddressToPatch + 1] = (absoluteJumpAddress >> 16) & 0xffff;

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
    if(check(parser, TokenType::LEFT_PAREN))
        return;
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

static void andFn(Parser& parser)
{
    i32 endJmp = emitJump(parser, OP_JUMP_IF_FALSE);
    emitByteCode(parser, OP_POP);

    parsePrecedence(parser, PREC_AND);
    patchJump(parser, endJmp);
}

static void orFn(Parser& parser)
{
    i32 endJmp = emitJump(parser, OP_JUMP_IF_TRUE);
    emitByteCode(parser, OP_POP);

    parsePrecedence(parser, PREC_OR);
    patchJump(parser, endJmp);
}

static void fnCall(Parser& parser)
{
    if(parser.previousPrevious.type != TokenType::IDENTIFIER)
    {
        errorAt(parser, parser.previousPrevious, "Expected function name identifier for calling function.");
    }

    std::string findStr = getStringFromTokenName(parser.previousPrevious);
    i32 functionIndex = -1;
    for(i32 i = 0; i < parser.script.functions.size(); ++i)
    {
        i32 realIndex = parser.script.functions[i].functionNameIndex;
        if(realIndex < parser.script.allSymbolNames.size() && parser.script.allSymbolNames[realIndex] == findStr)
        {
            functionIndex = i;
            break;
        }
    }
    if(functionIndex == -1)
    {
        std::string errStr = "Function ";
        errStr += findStr;
        errStr += " has not been declared.";
        errorAt(parser, parser.previousPrevious, errStr.c_str());
    }
    else
    {
        const Function& func = parser.script.functions[functionIndex];
        i32 currentAddress = parser.script.byteCode.size();
        emitByteCode(parser, OP_CONSTANT_I32);
        i32 constantAddressPosition = addConstant(parser.script, currentAddress, parser.previous.line);

        i32 paramCount = 0;
        if(!check(parser, TokenType::RIGHT_PAREN))
        {
            do {
                expression(parser);
                ++paramCount;

            } while(match(parser, TokenType::COMMA));

        }
        consume(parser, TokenType::RIGHT_PAREN, "Expected ')' after function arguments");
        if(func.functionParameterNameIndices.size() != paramCount)
        {
            std::string errTxt = "Expected parameter count: ";
            errTxt += std::to_string(func.functionParameterNameIndices.size());
            errTxt += " got ";
            errTxt += std::to_string(paramCount);
            errTxt += " parameters.";
            errorAtCurrent(parser, errTxt.c_str());
        }
        i32 jmpPoint = emitJump(parser, OP_JUMP_ADDRESS_DIRECTLY);
        parser.script.constants.structValueArray[constantAddressPosition] = parser.script.byteCode.size();
        parser.script.patchFunctions.push_back({.functionIndex = functionIndex, .addressToPatch = jmpPoint});
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

static void ifStatement(Parser& parser)
{
    consume(parser, TokenType::LEFT_PAREN, "Expect '(' after 'if'.");
    expression(parser);
    consume(parser, TokenType::RIGHT_PAREN, "Expect ')' after condition.");

    i32 thenJump = emitJump(parser, OP_JUMP_IF_FALSE);
    emitByteCode(parser, OP_POP);
    statement(parser);

    i32 elseJump = emitJump(parser, OP_JUMP);

    patchJump(parser, thenJump);
    emitByteCode(parser, OP_POP);

    if(match(parser, TokenType::ELSE))
    {
        statement(parser);
    }
    patchJump(parser, elseJump);
}

static void emitLoop(Parser& parser, i32 loopStart)
{
    emitByteCode(parser, OP_JUMP);
    i64 offset = loopStart - (i32)parser.script.byteCode.size() - 2;
    if(offset > INT32_MAX || offset < INT32_MIN)
    {
        error(parser, "Loop body jump too large.");
    }
    emitByteCode(parser, Op(offset & 0xffff));
    emitByteCode(parser, Op((offset >> 16) & 0xffff));
}

static void whileStatement(Parser& parser)
{
    i32 loopStart = (i32)parser.script.byteCode.size();
    consume(parser, TokenType::LEFT_PAREN, "Expect '(' after 'while'.");
    expression(parser);
    consume(parser, TokenType::RIGHT_PAREN, "Expect ')' after 'while' condition.");

    i32 exitJmp = emitJump(parser, OP_JUMP_IF_FALSE);
    emitByteCode(parser, OP_POP);
    statement(parser);
    emitLoop(parser, loopStart);

    patchJump(parser, exitJmp);
    emitByteCode(parser, OP_POP);
}





static void statement(Parser& parser)
{
    if(match(parser, TokenType::PRINT))
    {
        printStatement(parser);
    }
    else if(match(parser, TokenType::RETURN))
    {
        if(match(parser, TokenType::SEMICOLON))
        {
            emitReturn(parser);
        }
        else
        {
            expression(parser);
            consume(parser, TokenType::SEMICOLON, "Expected ';' after return expression.");
            emitByteCode(parser, OP_RETURN);
        }
    }
    else if(match(parser, TokenType::IF))
    {
        ifStatement(parser);
    }
    else if(match(parser, TokenType::WHILE))
    {
        whileStatement(parser);
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
        i32 realIndex = (i32)sta.structSymbolNameIndices[i];
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

static void fnDeclaration(Parser& parser)
{
    consume(parser, TokenType::IDENTIFIER, "Expect function name.");


    const StructStack& sta = parser.script.structStacks[parser.script.structIndex];
    if(sta.parentStructIndex != -1)
    {
        error(parser, "Expect fns to be only on top level.");
    }
    std::string str = getStringFromTokenName(parser.previous);


    i32 foundIndex = -1;
    for(i32 i = 0; i < parser.script.functions.size(); ++i)
    {
        i32 realIndex = (i32)parser.script.functions[i].functionNameIndex;
        if(realIndex < parser.script.allSymbolNames.size() && parser.script.allSymbolNames[realIndex] == str)
        {
            foundIndex = i;
            break;
        }
    }
    if(foundIndex >= 0 && parser.script.functions[foundIndex].defined)
    {
        std::string ss = "Function ";
        ss += str;
        ss += " has already been defined!";
        errorAt(parser, parser.previous, ss.c_str());
    }
    else
    {
        i32 symbolIndex = addSymbolName(parser.script, str.c_str());
        parser.script.functions.emplace_back();
        Function& func = parser.script.functions.back();
        func.functionNameIndex = symbolIndex;

        i32 jumpEnd = emitJump(parser, OP_JUMP);
        func.functionStartLocation = (i32)parser.script.byteCode.size();

        i32 parameters = 0;

        bool parsedParametersBefore = func.defined || func.declared;
        Token tokens[256];
        i32 tokenCount = 0;
        consume(parser, TokenType::LEFT_PAREN, "Expect '(' after function name.");
        if(!check(parser, TokenType::RIGHT_PAREN))
        {
            do
            {
                consume(parser, TokenType::IDENTIFIER, "Expected identifier for parameter name.");
                Token name = parser.previous;
                tokens[tokenCount++] = name;
                std::string paramName = getStringFromTokenName(name);

                i32 foundParamNameIndex = -1;
                for(i32 i = 0; i < func.functionParameterNameIndices.size(); ++i)
                {
                    i32 realIndex = func.functionParameterNameIndices[i];
                    if(realIndex < parser.script.allSymbolNames.size() && parser.script.allSymbolNames[realIndex] == paramName)
                    {
                        foundParamNameIndex = i;
                        break;
                    }
                }
                if(foundParamNameIndex != -1 && !parsedParametersBefore)

                {
                    errorAtCurrent(parser, "Parameter name already defined.");
                }
                consume(parser, TokenType::COLON, "Expected ':' and type for parameter.");
                ValueTypeDesc valueType{};
                if(match(parser, TokenType::I8))
                {
                    valueType = ValueTypeDesc{.valueType = ValueTypeI8 };
                }
                else if(match(parser,TokenType::I16))
                {
                    valueType = ValueTypeDesc{.valueType = ValueTypeI16 };
                }
                else if(match(parser, TokenType::I32))
                {
                    valueType = ValueTypeDesc{.valueType = ValueTypeI32 };
                }
                else if(match(parser, TokenType::U8))
                {
                    valueType = ValueTypeDesc{.valueType = ValueTypeU8 };
                }
                else if(match(parser,TokenType::U16))
                {
                    valueType = ValueTypeDesc{.valueType = ValueTypeU16 };
                }
                else if(match(parser, TokenType::U32))
                {
                    valueType = ValueTypeDesc{.valueType = ValueTypeU32 };
                }
                else if(match(parser,TokenType::F32))
                {
                    valueType = ValueTypeDesc{.valueType = ValueTypeF32 };
                }
                else if(match(parser,TokenType::F64))
                {
                    valueType = ValueTypeDesc{.valueType = ValueTypeF64 };
                }
                else if(check(parser, TokenType::COMMA))
                {
                    errorAtCurrent(parser, "Missing type for a parameter");
                    break;
                }
                else
                {
                    errorAtCurrent(parser, "Unknown type for a parameter");
                    break;
                }
                if(!parsedParametersBefore)
                {
                    func.functionParameterNameIndices.push_back(addSymbolName(parser.script, str.c_str()));
                    func.functionParamenterValueTypes.push_back(valueType);
                }
                else if(tokenCount > func.functionParameterNameIndices.size()
                    || tokenCount > func.functionParamenterValueTypes.size())
                {
                    errorAtCurrent(parser, "Parameter amount mismatched from previously defined.");
                }
                else if(func.functionParameterNameIndices[tokenCount - 1] != foundParamNameIndex)
                {
                    errorAtCurrent(parser, "Parameter names /order of names mismatched from previous define of function.");
                }
                else if(memcmp(&func.functionParamenterValueTypes[tokenCount - 1], &valueType, sizeof(valueType)) != 0)
                {
                    errorAtCurrent(parser, "Parameter names /order of names mismatched from previous define of function.");
                }


            } while(match(parser, TokenType::COMMA));
        }
        consume(parser, TokenType::RIGHT_PAREN, "Expect ')' after function parameters.");
        if(match(parser, TokenType::SEMICOLON))
        {
            if(func.declared && parameters != func.functionParameterNameIndices.size())
            {
                errorAtCurrent(parser, "Function declaration does not match parameter count.");

            }
            func.declared = true;
            return;
        }


        consume(parser, TokenType::LEFT_BRACE, "Expect '{' before function body.");

        beginScope(parser);
        for(int i = func.functionParameterNameIndices.size() - 1; i >= 0; --i)
        {

            i32 global = identifierConstant(parser, tokens[i]);

            defineVariable(parser, global);

        }
        block(parser);
        func.defined = true;

        endScope(parser);

        emitByteCode(parser, OP_RETURN);
        func.functionEndLocation = (i32)parser.script.byteCode.size();

        // Patch jump over to end of function.
        i32 offset = func.functionEndLocation - func.functionStartLocation;
        patchJump(parser, jumpEnd);

    }
}

static void declaration(Parser& parser)
{
    if(match(parser, TokenType::LET))
    {
        letDeclaration(parser);
    }
    else if(match(parser,TokenType::FN))
    {
        fnDeclaration(parser);
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

static void endCompiler(Parser& parser)
{
    for(const PatchFunctions& patchFns : parser.script.patchFunctions)
    {
        if(patchFns.addressToPatch < 0
            || patchFns.addressToPatch >= parser.script.byteCode.size()
            || patchFns.functionIndex < 0
            || patchFns.functionIndex >= parser.script.functions.size())
        {
            errorAtCurrent(parser, "Failed to patch function calls.");
        }
        else
        {
            const Function& fn = parser.script.functions[patchFns.functionIndex];
            patchJumpAbsolute(parser, patchFns.addressToPatch, fn.functionStartLocation);
        }
    }


#if DEBUG_PRINT_CODE
    if(!parser.hadError)
    {
        disassembleCode(parser.script, "code");
    }
#endif

    emitByteCode(parser, OP_CONSTANT_I32);
    addConstant(parser.script, 0, parser.previous.line);
    emitByteCode(parser, OP_RETURN);

    //emitReturn(parser);
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
