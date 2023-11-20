#include "nativefns.h"

#include "time.h"

NativeReturn clockNative(i32 argc, const TypeOfValue* values, const ValueTypeDesc* descs)
{
    assert(argc == 0);
    double cl = double(clock() / (double)CLOCKS_PER_SEC);
    TypeOfValue clRet = *((TypeOfValue *)&cl);
    return {
        .value = clRet,
        .desc = {.valueType = ValueTypeF64}
    };
}

