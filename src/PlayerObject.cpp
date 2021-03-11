#include "PlayerObject.h"
#include "ReplaySystem.h"
#include "PlayLayer.h"
#include "hook_utils.hpp"
#include "utils.hpp"

void PlayerObject::setup(uintptr_t base) {
    Hooks::addHook(base + 0x1F4F70, releaseButtonHook, &releaseButton);
}

void __fastcall PlayerObject::releaseButtonHook(void* self, void*, void* PlayerButton) {
    if (preventInput) return;
    releaseButton(self, PlayerButton);
}

namespace PlayerObject {
    float* getX(uintptr_t player) {
        return readPtr<float>(player + 0x67C);
    }

    double* getYAccel(uintptr_t player) {
        return readPtr<double>(player + 0x628);
    }

    float* getRotation(uintptr_t player) {
        return readPtr<float>(player + 0x20);
    }

    float* getSpriteRotation(uintptr_t player) {
        return readPtr<float>(player + 0x24);
    }
}