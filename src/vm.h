#pragma once

struct Script;


enum InterpretResult
{
    InterpretResult_Ok,
    InterpretResult_CompileError,
    InterpretResult_RuntimeError,

    InterpretResult_Count,
};


InterpretResult interpret(Script& script);
