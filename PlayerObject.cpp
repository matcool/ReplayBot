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
    if (preventInput) return;
    releaseButton(self, PlayerButton);
}

namespace PlayerObject {
    float* getX(uintptr_t player) {
        return reinterpret_cast<float*>(player + 0x67C);
    }

    double* getYAccel(uintptr_t player) {
        return reinterpret_cast<double*>(player + 0x628);
    }

    float* getRotation(uintptr_t player) {
        return reinterpret_cast<float*>(player + 0x20);
    }

    float* getSpriteRotation(uintptr_t player) {
        return reinterpret_cast<float*>(player + 0x24);
    }
}