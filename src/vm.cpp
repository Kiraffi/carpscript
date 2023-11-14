#include "vm.h"

#include "compiler.h"
#include "debug.h"
#include "mymemory.h"
#include "op.h"
#include "script.h"

#include <assert.h>
#include <stdarg.h> // va_start
#include <string.h> // memcpy

struct VMRuntime
{
    std::vector<TypeOfValue> &stack;
    const OpCodeType *codeStart;
    const OpCodeType *ip;
    const i32* lines;
    i32 line;
};

static bool truthy(TypeOfValue value)
{
    return value != 0;
}

struct HelperStruct
{
    TypeOfValue valueA;
    TypeOfValue valueB;
    ValueTypeDesc descA;
    ValueTypeDesc descB;
};

static HelperStruct valuesEqualHelper(std::vector<TypeOfValue>& stack, std::vector<ValueTypeDesc> &stackValueInfo)
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

static void valuesEqual(Script& script, std::vector<TypeOfValue>& stack, std::vector<ValueTypeDesc> &stackValueInfo)
{
    HelperStruct s = valuesEqualHelper(stack, stackValueInfo);
    bool isTrue = s.descB.valueType == s.descA.valueType;
    bool equal = isTrue && s.valueA == s.valueB;
    if(isTrue && s.descA.valueType == ValueTypeString)
    {
        equal = script.stackStrings[s.valueA] == script.stackStrings[s.valueB];
        script.stackStrings.pop_back();
        script.stackStrings.pop_back();
    }
    stack.push_back((equal) ? ~(0) : 0);
    stackValueInfo.push_back({.valueType = ValueTypeBool});

}

static size_t getInstructionIndex(const OpCodeType* current, const OpCodeType* start)
{
    return size_t(intptr_t(current) - intptr_t(start)) / OpCodeTypeSize;
}

static void resetStack(std::vector<TypeOfValue> &stack)
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

static TypeOfValue peekStack(const std::vector<TypeOfValue>& stack)
{
    return stack[stack.size() - 1];
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
static TypeOfValue doBinaryOpOp(const TypeOfValue l, const TypeOfValue r, OpCodeType opCode)
{
    const T& lt = *((const T*)&l);
    const T& rt = *((const T*)&r);
    TypeOfValue valueStackType = 0;
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
    TypeOfValue returnValue = *((TypeOfValue*)(&value));
    return returnValue;
}

static i32 doBinaryOp(
    std::vector<TypeOfValue>& stack,
    std::vector<ValueTypeDesc>& stackValueInfo,
    OpCodeType opCode)
{

    HelperStruct values = valuesEqualHelper(stack, stackValueInfo);

    assert(values.descA.valueType == values.descB.valueType);
    if(values.descA.valueType != values.descB.valueType)
    {
        return 1;
    }
    TypeOfValue finalValue;
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
    const i32 byteCodeSize = (i32)script.byteCode.size();
    const OpCodeType* ipStart = (const OpCodeType*) script.byteCode.data();
    const OpCodeType* ip = ipStart;

    const i32* lines = script.byteCodeLines.data();

    // Does stack need type info stack?
    u32 stackIndex = 0;
    std::vector<TypeOfValue> stack;
    std::vector<ValueTypeDesc> stackValueInfo;

    std::vector<u16> localValueAmounts;
    std::vector<i32> parentStructIndices;

    stack.reserve(1024 * 1024);
    stackValueInfo.reserve(1024 * 1024);

    i32& lastLocalAmount = script.previousLocalStartIndex;
    //i32 localDepth = 0;
    
    lastLocalAmount = 0;
    const std::vector<TypeOfValue> &valueArray = getCurrentStructStack(script).structValueArray;
    const std::vector<ValueTypeDesc> &valueTypeArray = getCurrentStructStack(script).structValueTypes;
    script.locals.structValueArray.insert(script.locals.structValueArray.end(), valueArray.begin(), valueArray.end());
    script.locals.structValueTypes.insert(script.locals.structValueTypes.end(), valueTypeArray.begin(), valueTypeArray.end());

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
        assert(stack.size() == stackValueInfo.size());
        assert(ip >= ipStart && ip < ipStart + byteCodeSize);
         OpCodeType opCode = *ip++;
        assert(ip >= ipStart && ip <= ipStart + byteCodeSize);
        vmRuntimeError.ip = ip;
        switch(opCode)
        {
            case OP_END_OF_FILE:
            {
                return InterpretResult_RuntimeError;
            }
            case OP_RETURN:
            {
                if(script.functionReturnAddresses.size() == 0)
                {
                    return InterpretResult_Ok;
                }
                i32 returnAddress = script.functionReturnAddresses.back();
                script.functionReturnAddresses.pop_back();

                if(returnAddress == 0 || returnAddress == byteCodeSize)
                {
                    printf("end Stack: %p value: %p\n", stack.data(), stackValueInfo.data());
                    return InterpretResult_Ok;
                }
                if(returnAddress < 0 || returnAddress > byteCodeSize)
                {
                    return InterpretResult_RuntimeError;
                }
                ip = ipStart + returnAddress;
                break;
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
                TypeOfValue* value = &script.constants.structValueArray[lookupIndex];
                stack.push_back(*value);
                ValueType type = ValueType(opCode - OP_CONSTANT_BOOL + ValueTypeBool);
                stackValueInfo.push_back(ValueTypeDesc{.valueType = type });
                break;
            }
            case OP_NOT:
            {
                TypeOfValue value = stack.back();
                stack.pop_back();

                stack.push_back(!truthy(value));
                stackValueInfo.pop_back();
                stackValueInfo.push_back({.valueType = ValueTypeBool});
                break;
            }
            case OP_CONSTANT_STRING:
            {
                u16 lookupIndex = *ip++;
                TypeOfValue value = script.constants.structValueArray[lookupIndex];

                const std::string& s = script.stringLiterals[value];

                i32 index = (i32)script.stackStrings.size();
                stack.push_back(index);

                script.stackStrings.push_back(s);

                stackValueInfo.push_back(ValueTypeDesc{.valueType = ValueTypeString});
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
                valuesEqual(script, stack, stackValueInfo);
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
                    TypeOfValue value = stack.back();
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
            case OP_PRINT:
            {
                TypeOfValue value = stack.back();
                stack.pop_back();

                ValueTypeDesc valueDesc = stackValueInfo.back();
                stackValueInfo.pop_back();

                printValue(script, &value, valueDesc.valueType);
                printf("\n");

                break;
            }
            case OP_POP:
            {
                stack.pop_back();
                stackValueInfo.pop_back();
                break;
            }
            case OP_DEFINE_GLOBAL:
            {
                u16 lookupIndex = *ip++;
                TypeOfValue *value = nullptr;
                //if(localDepth == 0)
                //{
                //    value = &getCurrentStructStack(script).structValueArray[lookupIndex];
                //}
                //else
                {
                    value = &script.locals.structValueArray[i32(lookupIndex) + lastLocalAmount]; //getCurrentStructStack(script).structValueArray[lookupIndex];
                }
                ValueTypeDesc* descA;

                if(!peek(stackValueInfo, 0, &descA))
                {
                    runtimeError(vmRuntimeError,
                                 "Trying to peek stack that does not have enough indices: %i", 1);
                    return InterpretResult_RuntimeError;
                }
                //if(localDepth == 0)
                //{
                //    getCurrentStructStack(script).structValueTypes[lookupIndex] = *descA;
                //}
                //else
                {
                    script.locals.structValueTypes[i32(lookupIndex) + lastLocalAmount] = *descA;
                }
                *value = stack.back();
                stack.pop_back();
                stackValueInfo.pop_back();
                break;
            }

            case OP_GET_GLOBAL:
            {
                i16 lookupIndex = i16(*ip++);
                TypeOfValue *value = nullptr;
                ValueTypeDesc *desc = nullptr;

                //if(localDepth == 0)
                //{
                //    value = &getCurrentStructStack(script).structValueArray[lookupIndex];
                //    desc = &getCurrentStructStack(script).structValueTypes[lookupIndex];
                //}
                //else
                {
                    value = &script.locals.structValueArray[i32(lookupIndex) + lastLocalAmount];
                    desc = &script.locals.structValueTypes[i32(lookupIndex) + lastLocalAmount];
                }
                stack.push_back(*value);
                stackValueInfo.push_back(*desc);
                break;
            }
            case OP_SET_GLOBAL:
            {
                i16 lookupIndex = i16(*ip++);
                TypeOfValue *value = nullptr;
                ValueTypeDesc *desc = nullptr;

                //if(localDepth == 0)
                //{
                //    value = &getCurrentStructStack(script).structValueArray[lookupIndex];
                //    desc = &getCurrentStructStack(script).structValueTypes[lookupIndex];
                //}
                //else
                {
                    value = &script.locals.structValueArray[i32(lookupIndex) + lastLocalAmount];
                    desc = &script.locals.structValueTypes[i32(lookupIndex) + lastLocalAmount];
                }
                *value = stack.back();
                stack.pop_back();

                ValueTypeDesc otherDesc = stackValueInfo.back();
                assert(desc->valueType == otherDesc.valueType);
                stackValueInfo.pop_back();


                stack.push_back(*value);
                stackValueInfo.push_back(*desc);

                if(desc->valueType != otherDesc.valueType)
                {
                    runtimeError(VMRuntime {.stack = stack, .codeStart = ipStart, .ip = ip,
                        .lines = lines, },
                        "Valuetypes mismatch for assignment: %i vs %i!", desc->valueType, otherDesc.valueType);
                    return InterpretResult_RuntimeError;

                }

                break;
            }
            case OP_STACK_SET:
            {
                u16 lookupIndex = *ip++;
                if(lookupIndex < 0 || lookupIndex >= (i32)script.structStacks.size())
                {
                    runtimeError(VMRuntime {.stack = stack, .codeStart = ipStart, .ip = ip,
                                     .lines = lines, },
                                 "Stack has no parent index!");
                    return InterpretResult_RuntimeError;
                }
                parentStructIndices.push_back(script.structIndex);
                script.structIndex = (i32)lookupIndex;
                localValueAmounts.push_back(lastLocalAmount);
                lastLocalAmount = script.locals.structValueArray.size();
                const std::vector<TypeOfValue>& valueArray = getCurrentStructStack(script).structValueArray;
                const std::vector<ValueTypeDesc>& valueTypeArray = getCurrentStructStack(script).structValueTypes;
                script.locals.structValueArray.insert(script.locals.structValueArray.end(), valueArray.begin(), valueArray.end());
                script.locals.structValueTypes.insert(script.locals.structValueTypes.end(), valueTypeArray.begin(), valueTypeArray.end());

                //localDepth++;
                break;
            }
            case OP_STACK_POP:
            {
                i32 parentIndex = parentStructIndices.back();
                parentStructIndices.pop_back();
                //i32 parentIndex = getCurrentStructStack(script).parentStructIndex;
                if(parentIndex < 0 || parentIndex >= (i32)script.structStacks.size())
                {
                    runtimeError(VMRuntime {.stack = stack, .codeStart = ipStart, .ip = ip,
                         .lines = lines, },
                     "Stack has no parent index!");
                    return InterpretResult_RuntimeError;
                }
                else
                {
                    script.structIndex = parentIndex;
                    //const std::vector<TypeOfValue> &valueArray = getCurrentStructStack(script).structValueArray;
                    //i32 currentSize = i32(script.locals.structValueArray.size());
                    script.locals.structValueArray.erase(script.locals.structValueArray.begin() + lastLocalAmount, script.locals.structValueArray.end());
                    script.locals.structValueTypes.erase(script.locals.structValueTypes.begin() + lastLocalAmount, script.locals.structValueTypes.end());



                    lastLocalAmount = localValueAmounts.back();
                    localValueAmounts.pop_back();
                    //localDepth--;
                    //assert(localDepth >= 0);
                }
                break;
            }
            case OP_JUMP_IF_FALSE:
            {
                i32 offset1 = *ip++;
                i32 offset2 = *ip++;
                i32 offset = offset1 | (offset2 << 16);
                if(!truthy(peekStack(stack)))
                {
                    ip += offset;
                }
                break;
            }
            case OP_JUMP_IF_TRUE:
            {
                i32 offset1 = *ip++;
                i32 offset2 = *ip++;
                i32 offset = offset1 | (offset2 << 16);
                if(truthy(peekStack(stack)))
                {
                    ip += offset;
                }
                break;
            }

            case OP_JUMP:
            {
                i32 offset1 = *ip++;
                i32 offset2 = *ip++;

                i32 offset = offset1 | (offset2 << 16);
                ip += offset;
                break;
            }

            case OP_JUMP_ADDRESS_DIRECTLY:
            {
                i32 address1 = *ip++;
                i32 address2 = *ip;

                i32 address = address1 | (address2 << 16);
                ip = ipStart + address;
                break;
            }
            case OP_CODE_PUSH_RETURN_ADDRESS:
            {
                i32 address1 = *ip++;
                i32 address2 = *ip++;

                i32 address = address1 | (address2 << 16);
                script.functionReturnAddresses.push_back(address);
                break;
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
                if(opCode == OP_ADD && descA->valueType == ValueTypeString)
                {
                    HelperStruct values = valuesEqualHelper(stack, stackValueInfo);
                    const std::string a = script.stackStrings[values.valueA];
                    const std::string b = script.stackStrings[values.valueB];

                    script.stackStrings.pop_back();
                    script.stackStrings.pop_back();

                    i32 newIndex = (i32)script.stackStrings.size();
                    script.stackStrings.push_back(a + b);

                    stack.push_back(newIndex);
                    stackValueInfo.push_back(ValueTypeDesc{.valueType = ValueTypeString});

                }
                else
                {
                    i32 result = doBinaryOp(stack, stackValueInfo, opCode);
                    if (result != 0)
                    {
                        switch (result)
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
                }
                break;
            }
            default:
            {
                printf("Unknown opcode runtime: %u\n", opCode);
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