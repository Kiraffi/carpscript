
#include "debug.h"
#include "mymemory.h"
#include "mytypes.h"
#include "op.h"
#include "script.h"

int main()
{
    MyMemory mem;
    Script& script = mem.scripts[addNewScript(mem)];

    addOpCode(script, OP_RETURN);

    addOpCode(script, OP_CONSTANT_I8);
    addConstant(script, 0, i8(-2));

    addOpCode(script, OP_CONSTANT_U8);
    addConstant(script, 0, u8(3));

    addOpCode(script, OP_CONSTANT_I32);
    addConstant(script, 0, i32(-18));

    addOpCode(script, OP_CONSTANT_I8);
    addConstant(script, 0, i8(-40));

    disassembleCode(script, "test op");

    return 0;
}
