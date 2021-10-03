#pragma once
#include "includes.h"
#include <vector>
#include <matdash.hpp>

namespace Hooks {
    void init();

    void CCScheduler_update(CCScheduler*, float dt);
    void CCKeyboardDispatcher_dispatchKeyboardMSG(CCKeyboardDispatcher*, int key, bool down);
    CCObject* CheckpointObject_create();
    
    namespace PlayLayer {
        void update(gd::PlayLayer*, float dt);

        void pushButton(gd::PlayLayer*, int, bool);
        void releaseButton(gd::PlayLayer*, int, bool);
        void resetLevel(gd::PlayLayer*);

        void pauseGame(gd::PlayLayer*, bool);
        
        void levelComplete(gd::PlayLayer*);
        // these are only for stopping recording/playing
        // maybe hook the destructor instead ?
        void onQuit(gd::PlayLayer*);

        void updateVisiblity(gd::PlayLayer*);
    }

    void PauseLayer_onEditor(gd::PauseLayer*, CCObject*);
    bool PauseLayer_init(gd::PauseLayer*);

    void PlayerObject_ringJump(gd::PlayerObject*, gd::GameObject* ring);
    void GameObject_activateObject(gd::GameObject*, gd::PlayerObject* player);
    void GJBaseGameLayer_bumpPlayer(gd::GJBaseGameLayer*, gd::PlayerObject* player, gd::GameObject* object);
}