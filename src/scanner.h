#pragma once

#include "mymemory.h"

struct Scanner
{
    MyMemory& mem;
    const u8* src;
    const u8* srcEnd;

    const u8* startToken;
    const u8* current;
    i32 line;
    bool hasErrors;
};
