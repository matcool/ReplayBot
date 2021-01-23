#include "pch.h"
#include "ReplaySystem.h"
#include "PauseLayer.h"
#include "PlayLayer.h"
#include "PlayerObject.h"
#include "GameManager.h"

void readInput(HMODULE hModule) {
    for (std::string line; std::getline(std::cin, line);) {
        if (line == "exit") {
            auto base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));
            PauseLayer::unload(base);
            PlayLayer::unload(base);
            PlayerObject::unload(base);
            fclose(stdout);
            fclose(stdin);
            FreeConsole();
            FreeLibraryAndExitThread(hModule, 0);
            break;
        }
        else if (line == "x") {
            auto x = reinterpret_cast<float*>(PlayLayer::getPlayer() + 0x67C);
            std::cout << *x << std::endl;
        }
        else if (line.rfind("fps ", 0) == 0) {
            auto fps = std::stoi(line.substr(4));
            std::cout << "Setting fps to " << fps << std::endl;
            ReplaySystem::getInstance()->setDefaultFPS(fps);
        }
        else if (line.rfind("speed ", 0) == 0) {
            auto speed = std::stof(line.substr(6));
            std::cout << "speedhack " << speed << std::endl;
            CCDirector::sharedDirector()->getScheduler()->setTimeScale(speed);
        }
    }
}

DWORD WINAPI my_thread(void* hModule) {
    MH_Initialize();
    
    AllocConsole();
    SetConsoleTitleA("Console");
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);

    auto base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));

    auto rs = ReplaySystem::getInstance();
    rs->init(base);
    PauseLayer::setup(base);
    PlayLayer::setup(base);
    PlayerObject::setup(base);
    GameManager::setup(base);

    MH_EnableHook(MH_ALL_HOOKS);

    std::cout << "ReplayBot loaded." << std::endl;
    std::cout << "FPS is set to " << rs->getDefaultFPS() << std::endl;
    std::cout << "To change it type in `fps (number)`" << std::endl;

    readInput(reinterpret_cast<HMODULE>(hModule));

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

