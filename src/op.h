#pragma once

#include "mytypes.h"

using OpCodeType = u16;
constexpr i32 OpCodeTypeSize = (i32)sizeof(OpCodeType);
static_assert(OpCodeTypeSize >= 2);
enum Op : OpCodeType
{
    OP_RETURN,
    OP_CONSTANT_I8 = 0x10,
    OP_CONSTANT_U8,
    OP_CONSTANT_I16,
    OP_CONSTANT_U16,
    OP_CONSTANT_I32,
    OP_CONSTANT_U32,
    OP_CONSTANT_I64,
    OP_CONSTANT_U64,
    OP_CONSTANT_F32,
    OP_CONSTANT_F64,


    OP_COUNT,
    OP_ERROR,
};