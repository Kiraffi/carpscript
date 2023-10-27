#include "script.h"

#include <assert.h>

template <typename T>
static i32 addConsantTemplate(Script& script, i32 structIndex, T constantValue, ValueType type)
{
    assert(structIndex >= 0 && structIndex == script.structDescs.size() - 1);

    u16 paramIndex = script.structDescs[structIndex].parametersCount++;
    paramIndex += script.structDescs[structIndex].parameterStartIndex;

    assert(paramIndex == script.structValueTypes.size());
    assert(paramIndex == script.structValueMemoryPosition.size());

    script.structValueTypes.emplace_back(ValueTypeDesc{.valueType = type});
    // "constant" should always be 0
    script.structSymbolNameIndices.emplace_back(addSymbolName(script, "constant"));

    i32 memPos = script.currentStructValuePos + sizeof(T) - 1;
    memPos &= ~(sizeof(T) - 1);

    script.structValueMemoryPosition.emplace_back(memPos);

    while(memPos + sizeof(T) > script.structValueArray.size())
    {
        script.structValueArray.emplace_back(0);
    }

    u8* p = &script.structValueArray[memPos];
    T* t = (T*)p;
    *t = constantValue;

    updateCurrentStructValuePos(script, memPos + sizeof(T));

    script.byteCode.emplace_back(memPos);
    return memPos;
}

void updateCurrentStructValuePos(Script& script, i32 value)
{
    assert(script.currentStructValuePos < value);
    script.currentStructValuePos = value;
    assert(script.currentStructValuePos < 65536);
}


i32 addOpCode(Script& script, Op op)
{
    script.byteCode.emplace_back(op);
    return (i32)script.byteCode.size() - 1;
}


i32 addSymbolName(Script& script, const char* name)
{
    i32 index = 0;
    while(index < script.allSymbolNames.size() && script.allSymbolNames[index] != name)
        index++;
    if(index == script.allSymbolNames.size())
    {
        script.allSymbolNames.emplace_back(name);
    }
    return index;
}


i32 addStruct(Script& script, const char* name)
{
    i32 index = (i32)script.structDescs.size();
    script.structDescs.emplace_back(StructDesc{});
    addSymbolName(script, name);
    return index;
}

i32 addConstant(Script& script, i32 structIndex, i8 constantValue)
{
    return addConsantTemplate(script, structIndex, constantValue, ValueTypeI8);
}
i32 addConstant(Script& script, i32 structIndex, u8 constantValue)
{
    return addConsantTemplate(script, structIndex, constantValue, ValueTypeU8);
}
i32 addConstant(Script& script, i32 structIndex, i16 constantValue)
{
    return addConsantTemplate(script, structIndex, constantValue, ValueTypeI16);
}
i32 addConstant(Script& script, i32 structIndex, u16 constantValue)
{
    return addConsantTemplate(script, structIndex, constantValue, ValueTypeU16);
}
i32 addConstant(Script& script, i32 structIndex, i32 constantValue)
{
    return addConsantTemplate(script, structIndex, constantValue, ValueTypeI32);
}
i32 addConstant(Script& script, i32 structIndex, u32 constantValue)
{
    return addConsantTemplate(script, structIndex, constantValue, ValueTypeU32);
}
i32 addConstant(Script& script, i32 structIndex, i64 constantValue)
{
    return addConsantTemplate(script, structIndex, constantValue, ValueTypeI64);
}
i32 addConstant(Script& script, i32 structIndex, u64 constantValue)
{
    return addConsantTemplate(script, structIndex, constantValue, ValueTypeU16);
}
i32 addConstant(Script& script, i32 structIndex, f32 constantValue)
{
    return addConsantTemplate(script, structIndex, constantValue, ValueTypeF32);
}
i32 addConstant(Script& script, i32 structIndex, f64 constantValue)
{
    return addConsantTemplate(script, structIndex, constantValue, ValueTypeF64);
}

