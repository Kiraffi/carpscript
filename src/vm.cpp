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

static bool truthy(StackType value)
{
    return value != 0;
}

struct HelperStruct
{
    StackType valueA;
    StackType valueB;
    ValueTypeDesc descA;
    ValueTypeDesc descB;
};

static HelperStruct valuesEqualHelper(std::vector<StackType>& stack, std::vector<ValueTypeDesc> &stackValueInfo)
{
    HelperStruct result;
    result.valueB = stack.back();
    stack.pop_back();
    result.valueA = stack.back();
    stack.pop_back();
    result.descB = stackValueInfo.back();
    stackValueInfo.pop_back();
    result.descA = stackValueInfo.back();
    stackValueInfo.pop_back();
    return result;
}

static bool valuesEqual(std::vector<StackType>& stack, std::vector<ValueTypeDesc> &stackValueInfo)
{
    HelperStruct s = valuesEqualHelper(stack, stackValueInfo);
    bool isTrue = s.descB.valueType == s.descA.valueType;
    stack.push_back((isTrue && s.valueA == s.valueB) ? ~(0) : 0);
    stackValueInfo.push_back({.valueType = ValueTypeBool});
}

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


static bool peek(std::vector<ValueTypeDesc>& descs, int distance, ValueTypeDesc** outDesc)
{
    if(distance >= descs.size())
    {
        outDesc = nullptr;
        return false;
    }
    *outDesc = &descs[descs.size() - distance - 1];
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

        case OP_GREATER: value = lt > rt; break;
        case OP_LESSER: value = lt < rt; break;
        default: break;
    }
    return value;
}

static i32 doBinaryOp(
    std::vector<StackType>& stack,
    std::vector<ValueTypeDesc>& stackValueInfo,
    OpCodeType opCode)
{

    HelperStruct values = valuesEqualHelper(stack, stackValueInfo);

    assert(values.descA.valueType == values.descB.valueType);
    if(values.descA.valueType != values.descB.valueType)
    {
        return 1;
    }
    StackType finalValue;
    ValueTypeDesc newDesc = values.descA;
    switch(values.descA.valueType)
    {
        case ValueTypeI8: finalValue = doBinaryOpOp<i8>(values.valueA, values.valueB, opCode); break;
        case ValueTypeU8: finalValue = doBinaryOpOp<u8>(values.valueA, values.valueB, opCode); break;
        case ValueTypeI16: finalValue = doBinaryOpOp<i16>(values.valueA, values.valueB, opCode); break;
        case ValueTypeU16: finalValue = doBinaryOpOp<u16>(values.valueA, values.valueB, opCode); break;
        case ValueTypeI32: finalValue = doBinaryOpOp<i32>(values.valueA, values.valueB, opCode); break;
        case ValueTypeU32: finalValue = doBinaryOpOp<u32>(values.valueA, values.valueB, opCode); break;
        case ValueTypeI64: finalValue = doBinaryOpOp<i64>(values.valueA, values.valueB, opCode); break;
        case ValueTypeU64: finalValue = doBinaryOpOp<u64>(values.valueA, values.valueB, opCode); break;
        case ValueTypeF32: finalValue = doBinaryOpOp<f32>(values.valueA, values.valueB, opCode); break;
        case ValueTypeF64: finalValue = doBinaryOpOp<f64>(values.valueA, values.valueB, opCode); break;
        default:
        {
            assert(false);
            return 2;
        }
    }
    switch(opCode)
    {
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
            stackValueInfo.emplace_back(values.descA);
            break;
        case OP_GREATER:
        case OP_LESSER:
            stackValueInfo.push_back({.valueType = ValueTypeBool});
            break;
        default:
            return 3;

    }
    stack.push_back(finalValue);

    return 0;
}

InterpretResult runCode(Script& script)
{
    const OpCodeType* ipStart = (const OpCodeType*) script.byteCode.data();
    const OpCodeType* ip = ipStart;

    const i32* lines = script.byteCodeLines.data();

    // Does stack need type info stack?
    u32 stackIndex = 0;
    std::vector<StackType> stack;
    std::vector<ValueTypeDesc> stackValueInfo;
    stack.reserve(1024 * 1024);
    stackValueInfo.reserve(1024 * 1024);

    VMRuntime vmRuntimeError = {
        .stack = stack,
        .codeStart = ipStart,
        .ip = ip,
        .lines = lines,
    };
    while(true)
    {
        #if DEBUG_TRACE_EXEC
            disassembleInstruction(script, i32(intptr_t(ip) - intptr_t(ipStart)) / OpCodeTypeSize);
        #endif
        OpCodeType opCode = *ip++;
        vmRuntimeError.ip = ip;
        switch(opCode)
        {
            case OP_END_OF_FILE:
            case OP_RETURN:
            {
                StackType value = stack.back();
                stack.pop_back();

                ValueTypeDesc valueDesc = stackValueInfo.back();
                stackValueInfo.pop_back();

                printValue(&value, valueDesc.valueType);
                printf("\n");
                return InterpretResult_Ok;
            }
            case OP_CONSTANT_BOOL:
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
                StackType* value = &script.structValueArray[lookupIndex];
                stack.push_back(*value);
                ValueType type = ValueType((opCode & 0xf) + ValueTypeBool);
                stackValueInfo.push_back(ValueTypeDesc{.valueType = type });
                break;
            }
            case OP_NOT:
            {
                StackType value = stack.back();
                stack.pop_back();

                stack.push_back(!truthy(value));
                stackValueInfo.pop_back();
                stackValueInfo.push_back({.valueType = ValueTypeBool});
                break;
            }
            case OP_NIL:
            {
                stackValueInfo.push_back(ValueTypeDesc{.valueType = ValueTypeNull});
                stack.push_back(0);
                break;
            }
            case OP_EQUAL:
            {
                valuesEqual(stack, stackValueInfo);
                break;
            }
            case OP_NEGATE:
            {
                ValueTypeDesc* desc;
                if(!peek(stackValueInfo, 0, &desc))
                {
                    runtimeError(vmRuntimeError,
                                 "Trying to peek stack that does not have enough indices: %i", 0);
                    return InterpretResult_RuntimeError;
                }
                if(desc->valueType >= ValueTypeI8 && desc->valueType <= ValueTypeF64)
                {
                    StackType value = stack.back();
                    stack.pop_back();
                    stack.push_back(-value);
                    break;
                }
                else
                {
                    const char* valueTypeName = "Unknown value type";
                    if(desc->valueType >= 0 && desc->valueType < ValueType::ValueTypeCount)
                    {
                        valueTypeName = ValueTypeNames[desc->valueType];
                    }
                    runtimeError(VMRuntime{.stack = stack, .codeStart = ipStart, .ip = ip,
                                     .lines = lines, },
                                 "Cannot negate the type: %i: %s", desc->valueType, valueTypeName);
                    return InterpretResult_RuntimeError;
                }
            }
            case OP_ADD:
            case OP_SUB:
            case OP_MUL:
            case OP_DIV:
            case OP_GREATER:
            case OP_LESSER:
            {
                ValueTypeDesc* descA;
                ValueTypeDesc* descB;
                if(!peek(stackValueInfo, 0, &descA) || !peek(stackValueInfo, 1, &descB))
                {
                    runtimeError(vmRuntimeError,
                                 "Trying to peek stack that does not have enough indices: %i", 1);
                    return InterpretResult_RuntimeError;
                }


                i32 result = doBinaryOp(stack, stackValueInfo, opCode);
                if(result != 0)
                {
                    switch(result)
                    {
                        case 1:
                            runtimeError(vmRuntimeError, "Mismatching types on binary op: %i vs %i!",
                                descA->valueType, descB->valueType);
                            break;
                        case 2:
                            runtimeError(vmRuntimeError, "Valuetype on binary op not a number: %i!",
                                descA->valueType);
                            break;
                        case 3:
                            runtimeError(vmRuntimeError, "Not valid binary op: %i!", opCode);
                            break;
                    }
                    return InterpretResult_RuntimeError;
                }
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