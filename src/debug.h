#pragma once

#include "common.h"
#include "mytypes.h"
#include "op.h"

struct Script;

void disassembleCode(const Script& script, const char* name);
i32 disassembleInstruction(const Script& script, i32 offset);

void printValue(const u8* value, ValueType type);
