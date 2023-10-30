#pragma once

#include "mytypes.h"
#include "op.h"
#include "token.h"

#include <vector>


#define DEBUG_PRINT_CODE 1
#define DEBUG_TRACE_EXEC 1

struct Script;

enum ValueType: u8
{
    ValueTypeNone,
    ValueTypeNull,

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

    ValueTypeStruct,
    ValueTypeString,

    ValueTypeCount,
};

static const char* ValueTypeNames[] =
{
    "ValueTypeNone",
    "ValueTypeNull",

    "ValueTypeBool",
    "ValueTypeI8",
    "ValueTypeU8",
    "ValueTypeI16",
    "ValueTypeU16",
    "ValueTypeI32",
    "ValueTypeU32",
    "ValueTypeI64",
    "ValueTypeU64",
    "ValueTypeF32",
    "ValueTypeF64",

    "ValueTypeStruct",

    "ValueTypeCount",

};

constexpr i32 getValueTypeSizeInBytes(ValueType type)
{
    switch(type)
    {
        case ValueTypeNone:
        case ValueTypeStruct:
        case ValueTypeCount:
            return 0;
        case ValueTypeBool:
        case ValueTypeI8:
        case ValueTypeU8:
            return 1;
        case ValueTypeI16:
        case ValueTypeU16:
            return 2;
        case ValueTypeI32:
        case ValueTypeU32:
        case ValueTypeF32:
            return 4;
        case ValueTypeI64:
        case ValueTypeU64:
        case ValueTypeF64:
            return 8;
    }
}

constexpr i32 getValueTypeSizeInOpCodes(ValueType type)
{
    i32 tmp = getValueTypeSizeInBytes(type) + OpCodeTypeSize - 1;

    return tmp  / OpCodeTypeSize;
}


struct ValueTypeDesc
{
    u16 structIndex; // If custom type like struct
    ValueType valueType;
    // How many *s, like *** = 3, direct value = 0
    u8 valueDerefLevel;
};


// Using linear memory, for example 12 functions before needed 47 parameters,
// parameterMemoryOffset = 47 and parametersCount could be 2.
// Next function would start with offset of 49.
struct FunctionDesc
{
    // Where to jump into in byte code.
    u32 functionMemoryStartLocation;
    u16 parameterStartIndex;
    u16 parametersCount;
};

struct StructDesc
{
    u16 parametersCount;
    // Assuming all structs require less than 64k parameters combined.
    u16 parameterStartIndex;
    // Might take several rounds to evaluate, if component structs.
    // Maybe require define in order. A should not have B inside if B has A.
    u32 structSize;
};

void printValue(const Script& script, const u64* value, ValueType type);
