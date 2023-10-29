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

InterpretResult runCode(Script& script);
InterpretResult interpret(MyMemory& mem, Script& script);
