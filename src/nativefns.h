#pragma once

#include "script.h"

NativeReturn clockNative(Script& script, i32 argc, const TypeOfValue* values, const ValueTypeDesc* descs);

NativeReturn addNative(Script& script, i32 argc, const TypeOfValue* values, const ValueTypeDesc* descs);

NativeReturn stringNative(Script& script, i32 argc, const TypeOfValue* values, const ValueTypeDesc* descs);
