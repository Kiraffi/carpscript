#pragma once

struct MyMemory;
struct Script;


enum InterpretResult
{
    InterpretResult_Ok,
    InterpretResult_CompileError,
    InterpretResult_RuntimeError,

    InterpretResult_Count,
};


InterpretResult interpret(MyMemory& mem, Script& script);
