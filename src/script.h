#pragma once

#include "common.h"
#include "mytypes.h"
#include "op.h"

#include <string>
#include <vector>

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

    // Level 0 struct is global
    std::vector<StructDesc> structDescs;
    std::vector<u32> structSymbolNameIndices;
    std::vector<ValueTypeDesc> structValueTypes;
    // Need to cast the values into type
    std::vector<u64> structValueArray;
    // For example u8 = 0, u8 = 1, u32 = 4 for first 3.
    std::vector<i32> structValueMemoryPosition;

    i32 currentStructValuePos;
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

i32 addStruct(Script& script, const char* name);

i32 addConstant(Script& script, i32 structIndex, i8 constantValue, i32 lineNumber);
i32 addConstant(Script& script, i32 structIndex, u8 constantValue, i32 lineNumber);
i32 addConstant(Script& script, i32 structIndex, i16 constantValue, i32 lineNumber);
i32 addConstant(Script& script, i32 structIndex, u16 constantValue, i32 lineNumber);
i32 addConstant(Script& script, i32 structIndex, i32 constantValue, i32 lineNumber);
i32 addConstant(Script& script, i32 structIndex, u32 constantValue, i32 lineNumber);
i32 addConstant(Script& script, i32 structIndex, i64 constantValue, i32 lineNumber);
i32 addConstant(Script& script, i32 structIndex, u64 constantValue, i32 lineNumber);
i32 addConstant(Script& script, i32 structIndex, f32 constantValue, i32 lineNumber);
i32 addConstant(Script& script, i32 structIndex, f64 constantValue, i32 lineNumber);

