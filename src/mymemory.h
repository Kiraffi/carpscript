#pragma once

#include "mytypes.h"

#include "script.h"



struct MyMemory
{
    std::vector<Script> scripts;

    std::vector<u8> scriptFile;
};

i32 addNewScript(MyMemory& mem);
