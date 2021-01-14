#include "pch.h"
#include "ReplaySystem.h"
#include "PauseLayer.h"
#include "PlayLayer.h"
#include "PlayerObject.h"

DWORD WINAPI my_thread(void* hModule) {
    MH_Initialize();
    
    AllocConsole();
    SetConsoleTitleA("Console");
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);

    auto base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));

    ReplaySystem::getInstance()->init(base);
    PauseLayer::setup(base);
    PlayLayer::setup(base);
    PlayerObject::setup(base);

    MH_EnableHook(MH_ALL_HOOKS);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0x1000, my_thread, hModule, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

