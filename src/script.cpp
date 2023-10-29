#include "script.h"

#include <assert.h>

template <typename T>
static i32 addConsantTemplate(
    Script& script,
    i32 structIndex,
    T constantValue,
    ValueType type,
    i32 lineNumber)
{
    assert(structIndex >= 0 && structIndex == script.structDescs.size() - 1);

    u16 paramIndex = script.structDescs[structIndex].parametersCount++;
    paramIndex += script.structDescs[structIndex].parameterStartIndex;

    assert(paramIndex == script.structValueTypes.size());
    assert(paramIndex == script.structValueMemoryPosition.size());
    assert(paramIndex == script.currentStructValuePos);
    assert(paramIndex == script.structSymbolNameIndices.size());

    script.structValueTypes.emplace_back(ValueTypeDesc{.valueType = type});
    // "constant" should always be 0
    script.structSymbolNameIndices.emplace_back(addSymbolName(script, "constant"));

    i32 memPos = script.currentStructValuePos;

    script.structValueMemoryPosition.emplace_back(memPos);

    while(memPos + 1 > script.structValueArray.size())
    {
        script.structValueArray.emplace_back(0);
    }

    u64* p = &script.structValueArray[memPos];
    T* t = (T*)p;
    *t = constantValue;

    updateCurrentStructValuePos(script, memPos + 1);

    script.byteCode.emplace_back(memPos);
    script.byteCodeLines.emplace_back(lineNumber);
    return memPos;
}

void updateCurrentStructValuePos(Script& script, i32 value)
{
    assert(script.currentStructValuePos < value);
    script.currentStructValuePos = value;
    assert(script.currentStructValuePos < 65536);
}


i32 addOpCode(Script& script, Op op, i32 lineNumber)
{
    script.byteCode.emplace_back(op);
    script.byteCodeLines.emplace_back(lineNumber);
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
    script.structIndex = index;
    addSymbolName(script, name);
    return index;
}

i32 addConstant(Script& script, i32 lineNumber)
{
    return addConsantTemplate(script, script.structIndex, nullptr, ValueTypeNull, lineNumber);
}

i32 addConstant(Script& script, bool constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, script.structIndex, constantValue, ValueTypeBool, lineNumber);
}
i32 addConstant(Script& script, i8 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, script.structIndex, constantValue, ValueTypeI8, lineNumber);
}
i32 addConstant(Script& script, u8 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, script.structIndex, constantValue, ValueTypeU8, lineNumber);
}
i32 addConstant(Script& script, i16 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, script.structIndex, constantValue, ValueTypeI16, lineNumber);
}
i32 addConstant(Script& script, u16 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, script.structIndex, constantValue, ValueTypeU16, lineNumber);
}
i32 addConstant(Script& script, i32 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, script.structIndex, constantValue, ValueTypeI32, lineNumber);
}
i32 addConstant(Script& script, u32 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, script.structIndex, constantValue, ValueTypeU32, lineNumber);
}
i32 addConstant(Script& script, i64 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, script.structIndex, constantValue, ValueTypeI64, lineNumber);
}
i32 addConstant(Script& script, u64 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, script.structIndex, constantValue, ValueTypeU16, lineNumber);
}
i32 addConstant(Script& script, f32 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, script.structIndex, constantValue, ValueTypeF32, lineNumber);
}
i32 addConstant(Script& script, f64 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, script.structIndex, constantValue, ValueTypeF64, lineNumber);
}

