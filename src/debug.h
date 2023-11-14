#pragma once

#include "common.h"
#include "mytypes.h"
#include "op.h"

struct Script;

void disassembleCode(Script& script, const char* name);
i32 disassembleInstruction(Script& script, i32 offset, bool executeStackInPop = false);

