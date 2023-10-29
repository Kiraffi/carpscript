#include "common.h"

#include <inttypes.h>
#include <stdio.h>

void printValue(const u64* value, ValueType type)
{
    switch(type)
    {
        case ValueTypeBool: printf("%s", *value ? "true" : "false"); break;
        case ValueTypeI8: printf("%" PRIi8, *((i8*)value)); break;
        case ValueTypeU8: printf("%" PRIu8, *((u8*)value)); break;
        case ValueTypeI16: printf("%" PRIi16, *((i16*)value)); break;
        case ValueTypeU16: printf("%" PRIu16, *((i16*)value)); break;
        case ValueTypeI32: printf("%" PRIi32, *((i32*)value)); break;
        case ValueTypeU32: printf("%" PRIu32, *((u32*)value)); break;
        case ValueTypeI64: printf("%" PRIi64, *((i64*)value)); break;
        case ValueTypeU64: printf("%" PRIu64, *((u64*)value)); break;
        case ValueTypeF32: printf("%f", *((f32*)value)); break;
        case ValueTypeF64: printf("%f", *((f64*)value)); break;

        case ValueTypeNull: printf("nil"); break;
        case ValueTypeStruct:
        case ValueTypeNone:
        case ValueTypeCount:
            printf("Not default type: %i", type);
    }
}
