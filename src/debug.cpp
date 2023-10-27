
#include "debug.h"

#include "common.h"
#include "op.h"
#include "script.h"

#include <inttypes.h>
#include <stdio.h>

void disassembleCode(const Script& script, const char* name)
{

    printf("== %s ==\n", name);
    i32 offset = 0;
    i32 length = script.byteCode.size();
    while(offset < length)
    {
        offset = disassembleInstruction(script, offset);
    }
}

static i32 simpleOpCode(const char* name, i32 offset)
{
    printf("%s\n", name);
    return offset + 1;
}

static i32 constantOpCode(const char* name, const Script& script, i32 offset, ValueType type)
{
    u16 lookupIndex = script.byteCode[offset + 1];


    printf("%-16s %4x '", name, lookupIndex);
    printValue(&script.structValueArray[lookupIndex], type);
    printf("\n");

    return offset + 1 + 1; // getValueTypeSizeInOpCodes(type);
}

i32 disassembleInstruction(const Script& script, i32 offset)
{
    printf("%04x ", offset);
    OpCodeType opCode = script.byteCode[offset];
    switch(opCode)
    {
        case OP_RETURN:
        {
            return simpleOpCode("OP_RETURN", offset);
        }

        case OP_CONSTANT_I8:
        case OP_CONSTANT_U8:
        case OP_CONSTANT_I16:
        case OP_CONSTANT_U16:
        case OP_CONSTANT_I32:
        case OP_CONSTANT_U32:
        case OP_CONSTANT_I64:
        case OP_CONSTANT_U64:
        case OP_CONSTANT_F32:
        case OP_CONSTANT_F64:
        {
            return constantOpCode("OP_CONSTANT", script, offset, ValueType((opCode & 0xf) + ValueTypeI8));
        }


        default:
        {
            printf("Unknown opcode %d\n", opCode);
            return offset + OpCodeTypeSize;
        }
    }
}

void printValue(const u8* value, ValueType type)
{
    switch(type)
    {
        case ValueTypeBool: printf("%s", *value ? "true" : "false"); break;
        case ValueTypeI8: printf("%" PRIi8, *((i8*)value)); break;
        case ValueTypeU8: printf("%" PRIu8, *value); break;
        case ValueTypeI16: printf("%" PRIi16, *((i16*)value)); break;
        case ValueTypeU16: printf("%" PRIu16, *((i16*)value)); break;
        case ValueTypeI32: printf("%" PRIi32, *((i32*)value)); break;
        case ValueTypeU32: printf("%" PRIu32, *((u32*)value)); break;
        case ValueTypeI64: printf("%" PRIi64, *((i64*)value)); break;
        case ValueTypeU64: printf("%" PRIu64, *((u64*)value)); break;
        case ValueTypeF32: printf("%f", *((f32*)value)); break;
        case ValueTypeF64: printf("%f", *((f64*)value)); break;

        case ValueTypeStruct:
        case ValueTypeNone:
        case ValueTypeCount:
            printf("Not default type: %i", type);
    }
}
