#pragma once
#include "includes.h"
#include <vector>

#define _THISCALL_HOOK(name, ret_type, self_type, ...) \
inline ret_type(__thiscall* name)(self_type* self, __VA_ARGS__); \
ret_type __fastcall name##_H(self_type* self, int, __VA_ARGS__);

#define _FASTCALL_HOOK(name, ret_type, ...) \
inline ret_type(__fastcall* name)(__VA_ARGS__); \
ret_type __fastcall name##_H(__VA_ARGS__);

namespace Hooks {
    void init();

    _THISCALL_HOOK(CCScheduler_update, void, CCScheduler, float dt)
    _THISCALL_HOOK(CCKeyboardDispatcher_dispatchKeyboardMSG, void, CCKeyboardDispatcher, int key, bool down)
    _FASTCALL_HOOK(CheckpointObject_create, CCObject*)
    
    namespace PlayLayer {
        _THISCALL_HOOK(update, void, gd::PlayLayer, float dt)

        _THISCALL_HOOK(pushButton, int, gd::PlayLayer, int, bool)
        _THISCALL_HOOK(releaseButton, int, gd::PlayLayer, int, bool)
        _THISCALL_HOOK(resetLevel, int, gd::PlayLayer)

        _THISCALL_HOOK(pauseGame, void, gd::PlayLayer, bool)
        
        _THISCALL_HOOK(levelComplete, void*, gd::PlayLayer)
        // these are only for stopping recording/playing
        // maybe hook the destructor instead ?
        _THISCALL_HOOK(onQuit, void*, gd::PlayLayer)
        _THISCALL_HOOK(onEditor, void*, gd::PlayLayer, void*)

        _THISCALL_HOOK(updateVisiblity, void, gd::PlayLayer)
    }

    _THISCALL_HOOK(PauseLayer_init, bool, gd::PauseLayer)

    _THISCALL_HOOK(PlayerObject_ringJump, void, gd::PlayerObject, gd::GameObject* ring)
    _THISCALL_HOOK(GameObject_activateObject, void, gd::GameObject, gd::PlayerObject* player)
    _THISCALL_HOOK(GJBaseGameLayer_bumpPlayer, void, gd::GJBaseGameLayer, gd::PlayerObject* player, gd::GameObject* object)
}