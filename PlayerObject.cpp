#include "PlayerObject.h"
#include "ReplaySystem.h"
#include "PlayLayer.h"

void PlayerObject::setup(uintptr_t base) {
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x1F4E40),
        pushButtonHook,
        reinterpret_cast<void**>(&pushButton)
    );
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x1F4F70),
        releaseButtonHook,
        reinterpret_cast<void**>(&releaseButton)
    );
}

void PlayerObject::unload(uintptr_t base) {
    MH_RemoveHook(reinterpret_cast<void*>(base + 0x1F4E40));
    MH_RemoveHook(reinterpret_cast<void*>(base + 0x1F4F70));
}

void __fastcall PlayerObject::pushButtonHook(void* self, void*, void* PlayerButton) {
    pushButton(self, PlayerButton);
}

void __fastcall PlayerObject::releaseButtonHook(void* self, void*, void* PlayerButton) {
    // pausing calls this function directly for some reason??
    // TODO: set preventInput to true on the function that calls this, doesnt seem to be on PauseLayer::init tho
    if (preventInput) return;
    releaseButton(self, PlayerButton);
}