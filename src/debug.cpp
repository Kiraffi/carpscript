
#include "debug.h"

#include "common.h"
#include "op.h"
#include "script.h"

void disassembleCode(const Script& script, const char* name)
{

    printf("== %s ==\n", name);
    i32 offset = 0;
    i32 length = (i32)script.byteCode.size();
    while(offset < length)
    {
        offset = disassembleInstruction(script, offset);
    }
    printf("== End of %s ==\n\n", name);
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

    printValue(script, &script.constants.structValueArray[lookupIndex], type);
    printf("'\n");

    return offset + 1 + 1; // getValueTypeSizeInOpCodes(type);
}
static i32 jumpInstruction(const char* name, const Script& script, i32 offset)
{
    i32 offset1 = script.byteCode[offset + 1];
    i32 offset2 = script.byteCode[offset + 2];
    i32 jump = offset1 | (offset2 << 16);
    printf("%-16s %4d -> %d\n", name, offset,
           offset + 3 + jump);
    return offset + 3;
}
static i32 globalVar(const char* name, const Script& script, i32 offset)
{
    u16 lookupIndex = script.byteCode[offset + 1];

    printf("%-16s %4x '", name, lookupIndex);

    printf("%s", script.allSymbolNames[getCurrentStructStack(script).structSymbolNameIndices[lookupIndex]].c_str());
    printf("'\n");

    return offset + 1 + 1; // getValueTypeSizeInOpCodes(type);
}


i32 disassembleInstruction(const Script& script, i32 offset)
{
    printf("%05x ", offset);
    if(offset > 0 && script.byteCodeLines[offset - 1] == script.byteCodeLines[offset])
    {
        printf("   | ");
    }
    else
    {
        printf("%4d ", script.byteCodeLines[offset]);
    }
    OpCodeType opCode = script.byteCode[offset];
    const char* opName = getOpCodeName(opCode);
    switch(opCode)
    {
        case OP_STACK_POP:
        case OP_POP:
        case OP_PRINT:
        case OP_END_OF_FILE:
        case OP_RETURN:
        case OP_NEGATE:
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_NIL:
        case OP_NOT:
        case OP_GREATER:
        case OP_LESSER:
        case OP_EQUAL:
            return simpleOpCode(opName, offset);

        case OP_STACK_SET:
        case OP_CONSTANT_STRING:
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
            return constantOpCode(opName, script, offset, ValueType((opCode & 0xff) + ValueTypeBool));
        }
        case OP_DEFINE_GLOBAL:
        case OP_GET_GLOBAL:
        case OP_SET_GLOBAL:
        {
            return globalVar(opName, script, offset);
        }
        case OP_JUMP:
        case OP_JUMP_IF_FALSE:
            return jumpInstruction(opName, script, offset);

        default:
        {
            printf("Unknown opcode debug: %d\n", opCode);
            return offset + OpCodeTypeSize;
        }
    }
}

