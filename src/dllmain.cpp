#include "includes.h"
#include "ReplaySystem.h"
#include "PauseLayer.h"
#include "PlayLayer.h"
#include "PlayerObject.h"
#include "GameManager.h"
#include <fstream>
#include "hook_utils.hpp"
#include "utils.hpp"
#include "overlay.hpp"

void readInput(HMODULE hModule) {
    for (std::string line; std::getline(std::cin, line);) {
        if (line == "exit") {
            auto base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));
            Hooks::unload();
            fclose(stdout);
            fclose(stdin);
            FreeConsole();
            FreeLibraryAndExitThread(hModule, 0);
            break;
        }
        else if (line == "x") {
            auto player = PlayLayer::getPlayer();
            if (player != nullptr)
                std::cout << player->m_xPos << " " << player->m_yPos << std::endl;
        }
        else if (line == "showcase") {
            auto s = ReplaySystem::getInstance()->toggleShowcaseMode();
            std::cout << "Showcase mode is now " << (s ? "ON" : "OFF") << std::endl;
        }
        else if (line.rfind("fps ", 0) == 0) {
            auto fps = std::stoi(line.substr(4));
            std::cout << "Setting fps to " << fps << std::endl;
            ReplaySystem::getInstance()->setDefaultFPS(fps);
        }
        else if (line.rfind("speed ", 0) == 0) {
            auto speed = std::stof(line.substr(6));
            auto replay = ReplaySystem::getInstance()->getCurrentReplay();
            if (!replay) continue;
            auto fps = replay->getFPS() * speed;
            std::cout << "Setting speed to " << speed << " (fps=" << fps << ")" << std::endl;
            CCDirector::sharedDirector()->setAnimationInterval(1. / fps);
        }
    }
}

DWORD WINAPI my_thread(void* hModule) {
    AllocConsole();
    SetConsoleTitleA("Console");
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);
    freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
    static std::ofstream conout("CONOUT$", std::ios::out);
    std::cout.rdbuf(conout.rdbuf());

    Hooks::init();

    auto base = reinterpret_cast<uintptr_t>(GetModuleHandle(0));

    auto rs = ReplaySystem::getInstance();
    rs->init(base);
    PauseLayer::setup(base);
    PlayLayer::setup(base);
    PlayerObjectHooks::setup(base);
    GameManager::setup(base);

    MH_EnableHook(MH_ALL_HOOKS);

    readInput(cast<HMODULE>(hModule));

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
        break;
    }
    return TRUE;
}

