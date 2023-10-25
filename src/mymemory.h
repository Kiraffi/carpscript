#pragma once

#include "mytypes.h"
#include "token.h"

#include <vector>

struct Parser
{
    std::vector<u8> code;
    std::vector<Token> tokens;
    u32 position;
    u32 line;
    u32 lineStart;
};

struct ScriptFile
{
    std::vector<u8> byteCode;


    std::vector<u8> stack;
    u32 bytecodePointer;
};

struct MyMemory
{
    std::vector<ScriptFile> scripts;

};