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

void handler(void* self, bool hold) {
    // maybe move all this logic to ReplaySystem?
    auto rs = ReplaySystem::getInstance();
    if (rs->isRecording()) {
        auto selfAddress = reinterpret_cast<uintptr_t>(self);
        auto x = reinterpret_cast<float*>(selfAddress + 0x67c);
        auto player2 = PlayLayer::getPlayer2();
        rs->getCurrentReplay()->addAction({ *x, hold, selfAddress == player2 });
    }
}

void __fastcall PlayerObject::pushButtonHook(void* self, void*, void* PlayerButton) {
    handler(self, true);
    if (ReplaySystem::getInstance()->isPlaying()) {
        return;
    }
    pushButton(self, PlayerButton);
}

void __fastcall PlayerObject::releaseButtonHook(void* self, void*, void* PlayerButton) {
    handler(self, false);
    if (ReplaySystem::getInstance()->isPlaying()) {
        return;
    }
    releaseButton(self, PlayerButton);
}