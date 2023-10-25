#pragma once

#include "mytypes.h"
#include "token.h"

#include <string>
#include <vector>


enum ValueType: u8
{
    ValueTypeNone,

    ValueTypeBool,
    ValueTypeI8,
    ValueTypeU8,
    ValueTypeI16,
    ValueTypeU16,
    ValueTypeI32,
    ValueTypeU32,
    ValueTypeI64,
    ValueTypeU64,
    ValueTypeF32,
    ValueTypeF64,

    ValueTypePtrI8,
    ValueTypePtrU8,
    ValueTypePtrI16,
    ValueTypePtrU16,
    ValueTypePtrI32,
    ValueTypePtrU32,
    ValueTypePtrI64,
    ValueTypePtrU64,
    ValueTypePtrF32,
    ValueTypePtrF64,

    ValueTypeStruct,
    ValueTypeStructPtr,

    ValueTypeCount,
};

struct ValueTypeDesc
{
    u16 structIndex; // If custom type like struct
    ValueType valueType;
};


struct Parser
{
    std::vector<u8> code;
    std::vector<Token> tokens;
    u32 position;
    u32 line;
    u32 lineStart;
};

// Using linear memory, for example 12 functions before needed 47 parameters,
// parameterMemoryOffset = 47 and parametersCount could be 2.
struct FunctionDesc
{
    // Where to jump into in byte code.
    u32 functionMemoryStartLocation;
    u16 parameterStartIndex;
    u16 parametersCount;
};

struct StructDesc
{
    // Assuming all structs require less than 64k parameters combined.
    u16 parameterStartIndex;
    u16 parametersCount;
    // Might take several rounds to evaluate, if component structs.
    // Maybe require define in order. A should not have B inside if B has A.
    u32 structSize;
};

struct ScriptFile
{
    std::vector<u8> byteCode;
    // Should have all identifiers.
    std::vector<std::string> allSymbolNames;

    std::vector<u8> globalValuesMemory; // memory for the offsets.
    std::vector<ValueTypeDesc> globalValueTypes;
    // For example u8 = 0, u8 = 1, u32 = 4 for first 3.
    std::vector<u32> globalValueMemoryOffsets;
    std::vector<u32> globalValueSymbolNameIndices;

    std::vector<FunctionDesc> functionDescs;
    // Every functionDesc has 0 to n amount of these, names and valuetypes.
    std::vector<u32> functionSymbolNameIndices;
    std::vector<ValueTypeDesc> functionValueTypes;

    std::vector<StructDesc> structDescs;
    std::vector<u32> structSymbolNameIndices;
    std::vector<ValueTypeDesc> structValueTypes;


    // Does stack need type info stack?
    std::vector<u8> stack;
    // such as
    std::vector<ValueTypeDesc> stackValueInfo;
    // does stack need symbol names from stack items
    std::vector<u32> stackSymbolNameIndices;

    u32 bytecodePointer;
};

struct MyMemory
{
    std::vector<ScriptFile> scripts;

};