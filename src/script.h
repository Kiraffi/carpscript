#pragma once

#include "common.h"
#include "mytypes.h"
#include "op.h"

#include <string>
#include <vector>

struct StructStack
{
    StructDesc desc;
    std::vector<u32> structSymbolNameIndices;
    std::vector<ValueTypeDesc> structValueTypes;
    std::vector<u64> structValueArray;
    i32 parentStructIndex;
};

struct Function
{
    // Which of the variable stacks we are using.
    i32 structStackIndex;
    i32 functionNameIndex;
    i32 functionStartLocation;
    i32 functionEndLocation;
    std::vector<i32> functionParameterNameIndices;
    std::vector<ValueTypeDesc> functionParamenterValueTypes;
    bool defined;
    bool declared;
};


struct PatchFunctions
{
    i32 functionIndex;
    i32 addressToPatch;
};

struct Script
{
    std::vector<OpCodeType> byteCode;
    std::vector<i32> byteCodeLines;
    // Should have all identifiers.
    std::vector<std::string> allSymbolNames;

    std::vector<FunctionDesc> functionDescs;
    // Every functionDesc has 0 to n amount of these, names and valuetypes.
    std::vector<u32> functionSymbolNameIndices;
    std::vector<ValueTypeDesc> functionValueTypes;

    std::vector<i32> functionReturnAddresses;

    std::vector<Function> functions;
    std::vector<PatchFunctions> patchFunctions;

    // Level 0 struct is global
    std::vector<StructStack> structStacks;

    StructStack constants;

    //std::vector<StructDesc> structDescs;
    //std::vector<u32> structSymbolNameIndices;
    //std::vector<ValueTypeDesc> structValueTypes;
    //// Need to cast the values into type
    //std::vector<u64> structValueArray;
    //// For example u8 = 0, u8 = 1, u32 = 4 for first 3.
    ////std::vector<i32> structValueMemoryPosition;

    std::vector<std::string> stringLiterals;
    std::vector<std::string> stackStrings;

    i32 structIndex;
};

//template<typename T>
//void writeBytes(Script& script, T data, i32 lineNumber)
//{
//    const u8* datePtr = (const u8*)&data;
//    const u8* dataPtrEnd = datePtr + sizeof(T);
//
//    script.byteCode.insert(script.byteCode.end(), datePtr, dataPtrEnd);
//    script.byteCodeLines.insert(script.byteCodeLines.end(), sizeof(T), lineNumber);
//}


void updateCurrentStructValuePos(Script& script, i32 increment);

i32 addOpCode(Script& script, Op op, i32 lineNumber);

i32 addSymbolName(Script& script, const char* name);

i32 addStruct(Script& script, const char* name, i32 parentIndex);

i32 addConstant(Script& script, i32 lineNumber);
i32 addConstant(Script& script, bool constValue, i32 lineNumber);
i32 addConstant(Script& script, i8 constantValue, i32 lineNumber);
i32 addConstant(Script& script, u8 constantValue, i32 lineNumber);
i32 addConstant(Script& script, i16 constantValue, i32 lineNumber);
i32 addConstant(Script& script, u16 constantValue, i32 lineNumber);
i32 addConstant(Script& script, i32 constantValue, i32 lineNumber);
i32 addConstant(Script& script, u32 constantValue, i32 lineNumber);
i32 addConstant(Script& script, i64 constantValue, i32 lineNumber);
i32 addConstant(Script& script, u64 constantValue, i32 lineNumber);
i32 addConstant(Script& script, f32 constantValue, i32 lineNumber);
i32 addConstant(Script& script, f64 constantValue, i32 lineNumber);

i32 addConstantString(Script& script, const std::string& str, i32 lineNumber);

static StructStack& getCurrentStructStack(Script& script)
{
    return script.structStacks[script.structIndex];
}
static const StructStack& getCurrentStructStack(const Script& script)
{
    return script.structStacks[script.structIndex];
}
