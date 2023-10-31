#include "mymemory.h"

i32 addNewScript(MyMemory& mem)
{
    Script s{};
    i32 index = (i32)mem.scripts.size();
    mem.scripts.emplace_back(s);
    Script& script = mem.scripts[index];
    script.allSymbolNames.emplace_back("constant");
    addStruct(script, "global", -1);
    return index;
}
