#include "vm.h"

#include "compiler.h"
#include "debug.h"
#include "mymemory.h"
#include "op.h"
#include "script.h"

#include <assert.h>
#include <stdarg.h> // va_start
#include <string.h> // memcpy

using StackType = u64;

struct VMRuntime
{
    std::vector<StackType> &stack;
    const OpCodeType *codeStart;
    const OpCodeType *ip;
    const i32* lines;
    i32 line;
};

static i32 getInstructionIndex(const OpCodeType* current, const OpCodeType* start)
{
    return i32(intptr_t(current) - intptr_t(start)) / OpCodeTypeSize;
}

static void resetStack(std::vector<StackType> &stack)
{
    stack.clear();
}

static void runtimeError(VMRuntime runtime, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputs("\n", stderr);

    size_t inst = getInstructionIndex(runtime.ip, runtime.codeStart) - 1;
    int line = runtime.lines[runtime.line];
    fprintf(stderr, "[Line %d] in script\n", line);
    resetStack(runtime.stack);
}

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


static bool peek(std::vector<ValueTypeDesc>& descs, int distance, ValueTypeDesc* outDesc)
{
    if(distance >= descs.size())
    {
        outDesc = nullptr;
        return false;
    }
    outDesc = &descs[descs.size() - distance - 1];
    return true;
}

template <typename T>
static StackType doBinaryOpOp(const StackType l, const StackType r, OpCodeType opCode)
{
    const T& lt = *((const T*)&l);
    const T& rt = *((const T*)&r);
    StackType valueStackType = 0;
    T& value = *((T*)&valueStackType);

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
    std::vector<StackType>& stack,
    std::vector<ValueTypeDesc>& stackValueInfo,
    OpCodeType opCode)
{
    StackType value2 = stack.back();
    stack.pop_back();
    StackType value = stack.back();
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

InterpretResult runCode(Script& script)
{
    const OpCodeType* ipStart = (const OpCodeType*) script.byteCode.data();
    const OpCodeType* ip = ipStart;

    const i32* lines = script.byteCodeLines.data();

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
                ValueTypeDesc* desc;
                if(!peek(stackValueInfo, 0, desc))
                {
                    runtimeError(VMRuntime{.stack = stack, .codeStart = ipStart, .ip = ip,
                                           .lines = lines, },
                                 "Trying to peek stack that does not have enough indices: %i", 0);
                    return InterpretResult_RuntimeError;
                }
                if(desc->valueType >= ValueTypeI8 && desc->valueType <= ValueTypeF64)
                {
                    u64 value = stack.back();
                    stack.pop_back();
                    stack.push_back(-value);
                    break;
                }
                else
                {
                    return InterpretResult_RuntimeError;
                }
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