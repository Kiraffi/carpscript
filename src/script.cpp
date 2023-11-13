#include "script.h"

#include <assert.h>

template <typename T>
static i32 addConsantTemplate(
    Script& script,
    T constantValue,
    ValueType type,
    i32 lineNumber)
{
    StructStack& s = script.constants;
    u16 paramIndex = s.desc.parametersCount++;
    paramIndex += s.desc.parameterStartIndex;

    //assert(paramIndex == s.structValueTypes.size());
    //assert(paramIndex == s.structSymbolNameIndices.size());

    s.structValueTypes.emplace_back(ValueTypeDesc{.valueType = type});
    // "constant" should always be 0
    s.structSymbolNameIndices.emplace_back(addSymbolName(script, "constant"));

    i32 memPos = (i32)s.structValueArray.size();

    while(memPos + 1 > s.structValueArray.size())
    {
        s.structValueArray.emplace_back(0);
    }

    TypeOfValue* p = &s.structValueArray[memPos];
    T* t = (T*)p;
    *t = constantValue;

    script.byteCode.emplace_back(memPos);
    script.byteCodeLines.emplace_back(lineNumber);
    return memPos;
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


i32 addStruct(Script& script, const char* name, i32 parentIndex)
{
    i32 index = (i32)script.structStacks.size();
    script.structStacks.emplace_back();
    script.structStacks[index].parentStructIndex = parentIndex;
    script.structIndex = index;
    addSymbolName(script, name);
    return index;
}

i32 addConstant(Script& script, i32 lineNumber)
{
    return addConsantTemplate(script, nullptr, ValueTypeNull, lineNumber);
}

i32 addConstant(Script& script, bool constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, constantValue, ValueTypeBool, lineNumber);
}
i32 addConstant(Script& script, i8 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, constantValue, ValueTypeI8, lineNumber);
}
i32 addConstant(Script& script, u8 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, constantValue, ValueTypeU8, lineNumber);
}
i32 addConstant(Script& script, i16 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, constantValue, ValueTypeI16, lineNumber);
}
i32 addConstant(Script& script, u16 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, constantValue, ValueTypeU16, lineNumber);
}
i32 addConstant(Script& script, i32 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, constantValue, ValueTypeI32, lineNumber);
}
i32 addConstant(Script& script, u32 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, constantValue, ValueTypeU32, lineNumber);
}
i32 addConstant(Script& script, i64 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, constantValue, ValueTypeI64, lineNumber);
}
i32 addConstant(Script& script, u64 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, constantValue, ValueTypeU16, lineNumber);
}
i32 addConstant(Script& script, f32 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, constantValue, ValueTypeF32, lineNumber);
}
i32 addConstant(Script& script, f64 constantValue, i32 lineNumber)
{
    return addConsantTemplate(script, constantValue, ValueTypeF64, lineNumber);
}


i32 addConstantString(Script& script, const std::string& str, i32 lineNumber)
{
    i32 stringIndex = (i32)script.stringLiterals.size();
    script.stringLiterals.push_back(str);
    i32 index = addConsantTemplate(script, stringIndex, ValueTypeStringLiteral, lineNumber);
    return index;

}
