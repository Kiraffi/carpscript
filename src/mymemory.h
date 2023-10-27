#pragma once

#include "mytypes.h"

#include "script.h"


struct MyMemory
{
    std::vector<Script> scripts;

};

i32 addNewScript(MyMemory& mem);
