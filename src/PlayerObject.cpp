#include "PlayerObject.h"
#include "ReplaySystem.h"
#include "PlayLayer.h"
#include "hook_utils.hpp"
#include "utils.hpp"

void PlayerObjectHooks::setup(uintptr_t base) {
    Hooks::addHook(base + 0x1F4F70, releaseButtonHook, &releaseButton);
}

void __fastcall PlayerObjectHooks::releaseButtonHook(PlayerObject* self, void*, void* PlayerButton) {
    if (preventInput) return;
    releaseButton(self, PlayerButton);
}