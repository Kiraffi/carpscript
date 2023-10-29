#include "vm.h"

#include "compiler.h"
#include "debug.h"
#include "mymemory.h"
#include "op.h"
#include "script.h"

#include <assert.h>
#include <string.h> // memcpy
#define DEBUG_TRACE_EXEC 1


template <typename T>
static T readConstant(const OpCodeType* ip, const Script& script)
{
    u16 lookupIndex = *ip;
    return (*(T*)&script.structValueArray[lookupIndex]);
}
template <typename T>
static T handleConstant(T value)
{

}

template <typename T>
static u64 doBinaryOpOp(const u64 l, const u64 r, OpCodeType opCode)
{
    const T& lt = *((const T*)&l);
    const T& rt = *((const T*)&r);
    u64 value64 = 0;
    T& value = *((T*)&value64);

    switch(opCode)
    {
        case OP_ADD: value = lt + rt; break;
        case OP_SUB: value = lt - rt; break;
        case OP_MUL: value = lt * rt; break;
        case OP_DIV: value = lt / rt; break;
        default: break;
    }
    return value;
}

static bool doBinaryOp(
    std::vector<u64>& stack,
    std::vector<ValueTypeDesc>& stackValueInfo,
    OpCodeType opCode)
{
    u64 value2 = stack.back();
    stack.pop_back();
    u64 value = stack.back();
    stack.pop_back();

    ValueTypeDesc valueDesc2 = stackValueInfo.back();
    stackValueInfo.pop_back();
    ValueTypeDesc valueDesc = stackValueInfo.back();
    assert(valueDesc.valueType == valueDesc2.valueType);
    if(valueDesc.valueType != valueDesc2.valueType)
    {
        return false;
    }
    u64 finalValue;
    switch(valueDesc.valueType)
    {
        case ValueTypeI8: finalValue = doBinaryOpOp<i8>(value, value2, opCode); break;
        case ValueTypeU8: finalValue = doBinaryOpOp<u8>(value, value2, opCode); break;
        case ValueTypeI16: finalValue = doBinaryOpOp<i16>(value, value2, opCode); break;
        case ValueTypeU16: finalValue = doBinaryOpOp<u16>(value, value2, opCode); break;
        case ValueTypeI32: finalValue = doBinaryOpOp<i32>(value, value2, opCode); break;
        case ValueTypeU32: finalValue = doBinaryOpOp<u32>(value, value2, opCode); break;
        case ValueTypeI64: finalValue = doBinaryOpOp<i64>(value, value2, opCode); break;
        case ValueTypeU64: finalValue = doBinaryOpOp<u64>(value, value2, opCode); break;
        case ValueTypeF32: finalValue = doBinaryOpOp<f32>(value, value2, opCode); break;
        case ValueTypeF64: finalValue = doBinaryOpOp<f64>(value, value2, opCode); break;
        default:
            assert(false);
            return false;
    }

    stack.push_back(finalValue);
    return true;
}

static InterpretResult runCode(Script& script)
{
    const OpCodeType* ipStart = (const OpCodeType*) script.byteCode.data();
    const OpCodeType* ip = ipStart;

    // Does stack need type info stack?
    u32 stackIndex = 0;
    std::vector<u64> stack;
    std::vector<ValueTypeDesc> stackValueInfo;
    stack.reserve(1024 * 1024);
    stackValueInfo.reserve(1024 * 1024);

    while(true)
    {
        #if DEBUG_TRACE_EXEC
            disassembleInstruction(script, i32(intptr_t(ip) - intptr_t(ipStart)) / OpCodeTypeSize);
        #endif
        OpCodeType opCode = *ip++;
        switch(opCode)
        {
            case OP_RETURN:
            {
                u64 value = stack.back();
                stack.pop_back();

                ValueTypeDesc valueDesc = stackValueInfo.back();
                stackValueInfo.pop_back();

                printValue(&value, valueDesc.valueType);

                return InterpretResult_Ok;
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
                u16 lookupIndex = *ip++;
                u64* value = &script.structValueArray[lookupIndex];
                stack.push_back(*value);
                ValueType type = ValueType((opCode & 0xf) + ValueTypeBool);
                stackValueInfo.push_back(ValueTypeDesc{.valueType = type });

                //printValue(value, ValueType((opCode & 0xf) + ValueTypeBool));
                //printf("\n");
                break;
            }
            case OP_NEGATE:
            {
                u64 value = stack.back();
                stack.pop_back();
                stack.push_back(-value);
                break;
            }
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            {
                doBinaryOp(stack, stackValueInfo, opCode);
                break;
            }
            default:
            {
                printf("Unknown opcode: %u\n", opCode);
                assert(false);
                break;
            }
        }
    }
}


InterpretResult interpret(MyMemory& mem, Script& script)
{
    if(!compile(mem, script))
    {
        return InterpretResult_CompileError;
    }

    return runCode(script);
}