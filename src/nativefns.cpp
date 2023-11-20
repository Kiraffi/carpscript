#include "nativefns.h"

#include "time.h"

NativeReturn clockNative(Script& script, i32 argc, const TypeOfValue* values, const ValueTypeDesc* descs)
{
    assert(argc == 0);
    double cl = double(clock() / (double)CLOCKS_PER_SEC);
    TypeOfValue clRet = *((TypeOfValue *)&cl);
    return {
        .value = clRet,
        .desc = {.valueType = ValueTypeF64}
    };
}

NativeReturn addNative(Script& script, i32 argc, const TypeOfValue* values, const ValueTypeDesc* descs)
{
    assert(argc == 2);
    assert(descs[0].valueType == ValueTypeF32);
    assert(descs[1].valueType == ValueTypeF32);
    f32 value1 = *((f32*)(values + 0));
    f32 value2 = *((f32*)(values + 1));
    u64 retVal;
    f32* retValF32 = (f32*)&retVal;
    *retValF32 = value1 + value2;
    return {
        .value = retVal,
        .desc = {.valueType = ValueTypeF32}
    };
}


NativeReturn stringNative(Script& script, i32 argc, const TypeOfValue* values, const ValueTypeDesc* descs)
{
    assert(argc == 2);
    script.stringLiterals.push_back("Testing some random string");

    u64 returnVal = script.stringLiterals.size() - 1;
    return {
        .value = returnVal,
        .desc = {.valueType = ValueTypeStringLiteral}
    };
}
