
#include "debug.h"
#include "mymemory.h"
#include "mytypes.h"
#include "op.h"
#include "script.h"
#include "vm.h"

int main()
{
    MyMemory mem;
    Script& script = mem.scripts[addNewScript(mem)];



    addOpCode(script, OP_CONSTANT_U8, 1);
    addConstant(script, 0, u8(3), 1);


    addOpCode(script, OP_CONSTANT_I8, 1);
    addConstant(script, 0, i8(-2), 1);

    addOpCode(script, OP_CONSTANT_I8, 4);
    addConstant(script, 0, i8(-40), 4);



    addOpCode(script, OP_CONSTANT_I32, 3);
    addConstant(script, 0, i32(-2), 3);

    addOpCode(script, OP_CONSTANT_I32, 3);
    addConstant(script, 0, i32(-18), 3);

    addOpCode(script, OP_CONSTANT_I32, 3);
    addConstant(script, 0, i32(-12), 3);

    addOpCode(script, OP_CONSTANT_I32, 3);
    addConstant(script, 0, i32(1), 3);


    addOpCode(script, OP_SUB, 0);
    addOpCode(script, OP_ADD, 0);
    addOpCode(script, OP_NEGATE, 0);

    addOpCode(script, OP_MUL, 0);

    addOpCode(script, OP_CONSTANT_I32, 3);
    addConstant(script, 0, i32(10), 3);
    addOpCode(script, OP_DIV, 0);

    addOpCode(script, OP_RETURN, 0);


    disassembleCode(script, "test op");

    interpret(script);

    return 0;
}
