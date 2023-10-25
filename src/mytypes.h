#pragma once

#include <ctype.h> // isalnum
#include <stdint.h>


using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;



static bool isAlphaNumUnderscore(char c)
{
    return isalnum(c) || c == '_';
}

#if _MSC_VER
#define DEBUG_BREAK_MACRO() __debugbreak()
#else
#include <signal.h>
//#define DEBUG_BREAK_MACRO __builtin_trap()
#define DEBUG_BREAK_MACRO(debugBreakExitIndex) raise(SIGTRAP); exit(debugBreakExitIndex);
#endif
