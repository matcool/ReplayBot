// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include "Replay.h"
#include "util.h"
#include "MenuLayer.h"

DWORD WINAPI my_thread(void* hModule) {
    MH_Initialize();

    AllocConsole();
    SetConsoleTitleA("TrashBot Console");
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);

    std::cout << "sup\n";

    uintptr_t base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));

    Replay::memInit(base);
    
    /*
    auto unload = [&]() {
        fclose(stdout);
        FreeConsole();
        MH_DisableHook(MH_ALL_HOOKS);
        FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(hModule), 0);
    };
    */

    MenuLayer::memInit(base, reinterpret_cast<HMODULE>(hModule), GetCurrentThread());

    MH_EnableHook(MH_ALL_HOOKS);

    std::cout << "Base: " << std::hex << base << std::endl;

    auto GameManager = follow(base + 0x3222D0);

    std::cout << "GameManager: " << std::hex << GameManager << std::endl;

    auto PlayLayer = follow(GameManager + 0x164);

    std::cout << "PlayLayer: " << std::hex << PlayLayer << std::endl;

    if (PlayLayer) {
        auto PlayerObject = follow(PlayLayer + 0x224);

        std::cout << "Player: " << std::hex << PlayerObject << std::endl;

        float* playerX = reinterpret_cast<float*>(PlayerObject + 0x34);

        std::cout << "player x: " << *playerX << std::endl;

        auto player2 = follow(PlayLayer + 0x228);
        std::cout << "player 2: " << std::hex << player2 << std::endl;
    }
    
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

