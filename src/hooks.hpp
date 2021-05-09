#pragma once
#include "includes.h"
#include "types.hpp"
#include <vector>

#define _THISCALL_HOOK(name, ret_type, self_type, ...) \
inline ret_type(__thiscall* name)(self_type* self, __VA_ARGS__); \
ret_type __fastcall name##_H(self_type* self, int, __VA_ARGS__);

namespace Hooks {
    void init();

    _THISCALL_HOOK(CCScheduler_update, void, CCScheduler, float dt)
    _THISCALL_HOOK(CCKeyboardDispatcher_dispatchKeyboardMSG, void, CCKeyboardDispatcher, int key, bool down)
    
    // _ cuz yeah
    namespace _PlayLayer {
        _THISCALL_HOOK(init, bool, PlayLayer, void* level)
        _THISCALL_HOOK(update, void, PlayLayer, float dt)

        _THISCALL_HOOK(pushButton, int, PlayLayer, int, bool)
        _THISCALL_HOOK(releaseButton, int, PlayLayer, int, bool)
        _THISCALL_HOOK(resetLevel, int, PlayLayer)

        _THISCALL_HOOK(pauseGame, void, PlayLayer, bool)
        
        _THISCALL_HOOK(createCheckpoint, int, PlayLayer)
        _THISCALL_HOOK(removeLastCheckpoint, void*, PlayLayer)

        _THISCALL_HOOK(levelComplete, void*, PlayLayer)
        // these are only for stopping recording/playing
        // maybe hook the destructor instead ?
        _THISCALL_HOOK(onQuit, void*, PlayLayer)
        _THISCALL_HOOK(onEditor, void*, PlayLayer, void*)
    }

    _THISCALL_HOOK(PauseLayer_init, bool, gd::PauseLayer)

    _THISCALL_HOOK(PlayerObject_ringJump, void, gd::PlayerObject, gd::GameObject* ring)
    _THISCALL_HOOK(GameObject_activateObject, void, gd::GameObject, gd::PlayerObject* player)
}