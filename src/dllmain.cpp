#include "includes.h"
#include <fstream>
#include <matdash.hpp>
#include <matdash/boilerplate.hpp>

#include "hooks.hpp"

void mod_main(HMODULE module) {
#ifdef SHOW_CONSOLE
    AllocConsole();
    std::ofstream conout("CONOUT$", std::ios::out);
    std::ifstream conin("CONIN$", std::ios::in);
    std::cout.rdbuf(conout.rdbuf());
    std::cin.rdbuf(conin.rdbuf());
#endif

    Hooks::init();

#ifdef SHOW_CONSOLE
    std::getline(std::cin, std::string());
    MH_Uninitialize();
    conout.close();
    conin.close();
    FreeConsole();
    FreeLibraryAndExitThread(cast<HMODULE>(module), 0);
#endif
}
