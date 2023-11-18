#include <stdlib.h>
#include <string.h>

#include <vector>

#include "error.h"
#include "mymemory.h"
#include "mytypes.h"
#include "vm.h"


static bool runFile(const char* filename)
{
    printf("Filename: %s\n", filename);

    if(filename == nullptr)
    {
        LOG_ERROR("Filename is nullptr");
        return false;
    }

    FILE* file = fopen(filename, "rb");
    if(file == nullptr)
    {
        LOG_ERROR("Failed to open file.");
        return false;
    }
    MyMemory mem{};

    fseek(file, 0L, SEEK_END);
    size_t sz = ftell(file);
    fseek(file, 0L, SEEK_SET);

    Script& script = mem.scripts[addNewScript(mem)];

    mem.scriptFile.resize(sz + 1);
    fread(mem.scriptFile.data(), 1, sz, file);
    mem.scriptFile[sz] = '\0';
    fclose(file);

    InterpretResult result = interpret(mem, script);

    switch(result)
    {
        case InterpretResult_CompileError:
            printf("Failed to compile: %s\n", filename);
            break;
        case InterpretResult_Ok:
        default:
            break;
    }
    {
        /*
        // printf("%s\n", mem.scriptFileData.data());
        if(ast_generate(mem))
        {
            for(i32 index : mem.blocks[0].statementIndices)
            {
                const Statement& statement = mem.statements[index];
                interpret(mem, statement);
            }
        }
         */
    }


    return true;
}



static void runPrompt()
{
}

int main(int argc, const char** argv)
{
    if(argc > 3)
    {
        printf("Usage: carp [script]\n");
        return 64;
    }
    else if(argc == 2)
    {
        if(!runFile(argv[1]))
        {
            printf("Failed to run file: %s\n", argv[1]);
        }
    }
    else
    {
        const char* filename = "prog/recu.carp";
        if(!runFile(filename))
        {
            printf("Failed to run file: %s\n", filename);
        }
        printf("Script scanned!\n");
        //runPrompt();
    }
    //printf("Wait until enter pressed!\n");
    //char tmp;
    //scanf("%c", &tmp);
    return 0;
}
