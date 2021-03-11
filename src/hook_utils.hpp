#ifndef __HOOK_UTILS_HPP
#define __HOOK_UTILS_HPP
#include "includes.h"

namespace Hooks {
    void init();
    void unload();
    void addHook(uintptr_t address, void* hook, void* original);
    void addHook(void* address, void* hook, void* original);
}
#endif