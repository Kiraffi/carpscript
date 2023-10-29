#pragma once

#include "mytypes.h"

#include <assert.h>

using OpCodeType = u16;
constexpr i32 OpCodeTypeSize = (i32)sizeof(OpCodeType);
static_assert(OpCodeTypeSize >= 2);
enum Op : OpCodeType
{
    OP_RETURN,
    OP_NEGATE,

    OP_CONSTANT_BOOL = 0x100,
    OP_CONSTANT_I8,
    OP_CONSTANT_U8,
    OP_CONSTANT_I16,
    OP_CONSTANT_U16,
    OP_CONSTANT_I32,
    OP_CONSTANT_U32,
    OP_CONSTANT_I64,
    OP_CONSTANT_U64,
    OP_CONSTANT_F32,
    OP_CONSTANT_F64,

    OP_ADD = 0x200,
    OP_SUB,
    OP_MUL,
    OP_DIV,

    OP_ERROR,
};

static const char* getOpCodeName(OpCodeType type)
{
    switch(type)
    {
        case OP_RETURN: return "OP_RETURN";
        case OP_NEGATE: return "OP_NEGATE";
        case OP_CONSTANT_BOOL: return "OP_CONSTANT_BOOL";
        case OP_CONSTANT_I8: return "OP_CONSTANT_I8";
        case OP_CONSTANT_U8: return "OP_CONSTANT_U8";
        case OP_CONSTANT_I16: return "OP_CONSTANT_I16";
        case OP_CONSTANT_U16: return "OP_CONSTANT_U16";
        case OP_CONSTANT_I32: return "OP_CONSTANT_I32";
        case OP_CONSTANT_U32: return "OP_CONSTANT_U32";
        case OP_CONSTANT_I64: return "OP_CONSTANT_I64";
        case OP_CONSTANT_U64: return "OP_CONSTANT_U64";
        case OP_CONSTANT_F32: return "OP_CONSTANT_F32";
        case OP_CONSTANT_F64: return "OP_CONSTANT_F64";

        case OP_ADD: return "OP_ADD";
        case OP_SUB: return "OP_SUB";
        case OP_MUL: return "OP_MUL";
        case OP_DIV: return "OP_DIV";

        case OP_ERROR: return "OP_ERROR";
        default:
            assert(false);
    }
}