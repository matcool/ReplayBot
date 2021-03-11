#include "hook_utils.hpp"
#include "utils.hpp"

namespace Hooks {
    void init() {
        MH_Initialize();
    }
    void unload() {
        MH_Uninitialize();
    }
    void addHook(uintptr_t address, void* hook, void* original) {
        addHook(cast<void*>(address), hook, original);
    }
    void addHook(void* address, void* hook, void* original) {
        MH_CreateHook(address, hook, cast<void**>(original));
    }
}