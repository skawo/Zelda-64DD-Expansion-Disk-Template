#include "diskCode.h"

typedef struct FuncReplacement
{
    void** toReplace;
    void* replaceWith;
} FuncReplacement;

// Define a function stub as __target_func that wraps the target_func.
// Create a marker label immediately after it in memory.
#define STUB_FUNC(target_func)                      \
    extern void __##target_func(void);              \
    __asm__(                                        \
        ".global __" #target_func "\n"              \
        ".type __" #target_func ", @function\n"     \
        "__" #target_func ":\n"                     \
        "    j " #target_func "\n"                  \
        "    nop\n"                                 \
    )   // Size is aways 8

#define FUNC_REPL_ENTRY(replacedFunction, newFunction)      \
{                                                           \
    .toReplace = (void**)&replacedFunction,                 \
    .replaceWith = (void*)&__##newFunction,                 \
}

#define DD_FUNC_REPLACEMENTS(...)         \
    FuncReplacement replFunctions[] = {   \
        __VA_ARGS__                       \
    }
