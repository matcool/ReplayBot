#include "includes.h"
#include <fstream>

#include "hooks.hpp"

#define _DEBUG

DWORD WINAPI thread_entry(void* module) {
#ifdef _DEBUG
    AllocConsole();
    static std::ofstream conout("CONOUT$", std::ios::out);
    static std::ifstream conin("CONIN$", std::ios::in);
    std::cout.rdbuf(conout.rdbuf());
    std::cin.rdbuf(conin.rdbuf());
#endif

    MH_Initialize();

    Hooks::init();

    MH_EnableHook(MH_ALL_HOOKS);

#ifdef _DEBUG
    std::getline(std::cin, std::string());
    MH_Uninitialize();
    conout.close();
    conin.close();
    FreeConsole();
    FreeLibraryAndExitThread(cast<HMODULE>(module), 0);
#endif

    return 0;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        auto handle = CreateThread(0, 0, thread_entry, module, 0, 0);
        if (handle) CloseHandle(handle);
    }
    return TRUE;
}