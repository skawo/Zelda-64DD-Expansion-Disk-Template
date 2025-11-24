#include "diskCode.h"

typedef struct FuncReplacement
{
    void** toReplace;
    void* replaceWith;
} FuncReplacement;

#define FUNC_REPL_ENTRY(replacedFunction, newFunction)      \
{                                                           \
    .toReplace = (void**)&replacedFunction,                 \
    .replaceWith = (void*)&newFunction,                     \
}

#define DD_FUNC_REPLACEMENTS(...)         \
    FuncReplacement replFunctions[] = {   \
        __VA_ARGS__                       \
    }
