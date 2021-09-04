#include "hooks.hpp"
#include "replay_system.hpp"
#include "overlay_layer.hpp"
#include "recorder.hpp"
#include <chrono>

auto add_gd_hook(uintptr_t offset, void* hook, void* orig) {
    return MH_CreateHook(cast<void*>(gd::base + offset), hook, cast<void**>(orig));
}
#define ADD_GD_HOOK(o, f) add_gd_hook(o, f##_H, &f)

auto add_cocos_hook(const char* symbol, void* hook, void* orig) {
    return MH_CreateHook(GetProcAddress(GetModuleHandleA("libcocos2d.dll"), symbol), hook, cast<void**>(orig));
}
#define ADD_COCOS_HOOK(o, f) add_cocos_hook(o, f##_H, &f)

void Hooks::init() {
    ADD_COCOS_HOOK("?update@CCScheduler@cocos2d@@UAEXM@Z", CCScheduler_update);
    ADD_COCOS_HOOK("?dispatchKeyboardMSG@CCKeyboardDispatcher@cocos2d@@QAE_NW4enumKeyCodes@2@_N@Z", CCKeyboardDispatcher_dispatchKeyboardMSG);
    ADD_GD_HOOK(0x20ddd0, CheckpointObject_create);

    ADD_GD_HOOK(0x1FB780, PlayLayer::init);
    ADD_GD_HOOK(0x2029C0, PlayLayer::update);

    ADD_GD_HOOK(0x111500, PlayLayer::pushButton);
    ADD_GD_HOOK(0x111660, PlayLayer::releaseButton);
    ADD_GD_HOOK(0x20BF00, PlayLayer::resetLevel);

    ADD_GD_HOOK(0x20D3C0, PlayLayer::pauseGame);

    ADD_GD_HOOK(0x1FD3D0, PlayLayer::levelComplete);
    ADD_GD_HOOK(0x20D810, PlayLayer::onQuit);
    ADD_GD_HOOK(0x1E60E0, PlayLayer::onEditor);

    ADD_GD_HOOK(0x205460, PlayLayer::updateVisiblity);

    ADD_GD_HOOK(0x1E4620, PauseLayer_init);

    ADD_GD_HOOK(0x1f4ff0, PlayerObject_ringJump);
    ADD_GD_HOOK(0xef0e0, GameObject_activateObject);
    ADD_GD_HOOK(0x10ed50, GJBaseGameLayer_bumpPlayer);
}

// yes these are global, too lazy to store them in replaysystem or smth
// not like theyre used anywhere else atm
bool g_disable_render = false;
float g_left_over = 0.f;

void __fastcall Hooks::CCScheduler_update_H(CCScheduler* self, int, float dt) {
    auto& rs = ReplaySystem::get_instance();
    if (rs.recorder.m_recording || rs.is_playing() || rs.is_recording() && gd::GameManager::sharedState()->getPlayLayer()) {
        const auto fps = rs.get_replay().get_fps();
        auto speedhack = self->getTimeScale();

        const float target_dt = 1.f / fps / speedhack;

        if (!rs.real_time_mode)
            return CCScheduler_update(self, target_dt);

        // todo: find ways to disable more render stuff
        g_disable_render = false;

        // TODO: not have this min()
        // doing the commented out if below causes really weird stutters for some reason
        // const int times = min(static_cast<int>((dt + g_left_over) / target_dt), 150);
        unsigned times = static_cast<int>((dt + g_left_over) / target_dt);
        // if the fps is really low then dont run it a lot of times
        // if (dt > 1.f / 10.f) {
        //     times = min(times, 100);
        // }
        auto start = std::chrono::high_resolution_clock::now();
        for (unsigned i = 0; i < times; ++i) {
            // if (i == times - 1)
            //     g_disable_render = false;
            CCScheduler_update(self, target_dt);
            using namespace std::literals;
            if (std::chrono::high_resolution_clock::now() - start > 16.666ms) {
                times = i + 1;
                break;
            }
        }
        g_left_over += dt - target_dt * times;
    } else {
        CCScheduler_update(self, dt);
    }
}

void __fastcall Hooks::CCKeyboardDispatcher_dispatchKeyboardMSG_H(CCKeyboardDispatcher* self, int, int key, bool down) {
    auto& rs = ReplaySystem::get_instance();
    if (down) {
        auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
        if (rs.is_recording() && play_layer) {
            if (key == 'C') {
                rs.set_frame_advance(false);
                PlayLayer::update_H(play_layer, 0, 1.f / rs.get_default_fps());
                rs.set_frame_advance(true);
            } else if (key == 'F') {
                rs.set_frame_advance(false);
            } else if (key == 'R') {
                PlayLayer::resetLevel_H(play_layer, 0);
            }
        }
    }
    CCKeyboardDispatcher_dispatchKeyboardMSG(self, key, down);
}

bool __fastcall Hooks::PlayLayer::init_H(gd::PlayLayer* self, int, void* level) {
    return init(self, level);
}

void __fastcall Hooks::PlayLayer::update_H(gd::PlayLayer* self, int, float dt) {
    auto& rs = ReplaySystem::get_instance();
    if (rs.get_frame_advance()) return;
    if (rs.is_playing()) rs.handle_playing();
    if (rs.recorder.m_recording) {
        // is menu thing open
        if (!from_offset<bool>(self, 0x4BD)) {
            auto frame_dt = 1.f / static_cast<float>(rs.recorder.m_fps);
            auto time = self->m_time + rs.recorder.m_extra_t - rs.recorder.m_last_frame_t;
            if (time >= frame_dt) {
                rs.recorder.m_extra_t = time - frame_dt;
                rs.recorder.m_last_frame_t = self->m_time;
                rs.recorder.capture_frame();
            }
        } else {
            rs.recorder.stop();
        }
    }
    update(self, dt);
}


bool _player_button_handler(bool hold, bool button) {
    if (gd::GameManager::sharedState()->getPlayLayer()) {
        auto& rs = ReplaySystem::get_instance();
        if (rs.is_playing()) return true;
        rs.record_action(hold, button);
    }
    return false;
}

int __fastcall Hooks::PlayLayer::pushButton_H(gd::PlayLayer* self, int, int idk, bool button) {
    if (_player_button_handler(true, button)) return 0;
    return pushButton(self, idk, button);
}

int __fastcall Hooks::PlayLayer::releaseButton_H(gd::PlayLayer* self, int, int idk, bool button) {
    if (_player_button_handler(false, button)) return 0;
    return releaseButton(self, idk, button);
}

int __fastcall Hooks::PlayLayer::resetLevel_H(gd::PlayLayer* self, int) {
    auto ret = resetLevel(self);
    ReplaySystem::get_instance().on_reset();
    return ret;
}


void __fastcall Hooks::PlayLayer::pauseGame_H(gd::PlayLayer* self, int, bool idk) {
    auto addr = cast<void*>(gd::base + 0x20D43C);
    auto& rs = ReplaySystem::get_instance();
    if (rs.is_recording())
        rs.record_action(false, true, false);

    bool should_patch = rs.is_playing();
    if (should_patch)
        patch(addr, {0x83, 0xC4, 0x04, 0x90, 0x90});
    
    pauseGame(self, idk);

    if (should_patch)
        patch(addr, {0xe8, 0x2f, 0x7b, 0xfe, 0xff});
}


CCObject* __fastcall Hooks::CheckpointObject_create_H() {
    return CheckpointObjectMod::create();
}

void* __fastcall Hooks::PlayLayer::levelComplete_H(gd::PlayLayer* self, int) {
    ReplaySystem::get_instance().reset_state();
    return levelComplete(self);
}

void _on_exit_level() {
    auto& rs = ReplaySystem::get_instance();
    rs.reset_state();
}

void* __fastcall Hooks::PlayLayer::onQuit_H(gd::PlayLayer* self, int) {
    _on_exit_level();
    return onQuit(self);
}

void* __fastcall Hooks::PlayLayer::onEditor_H(gd::PlayLayer* self, int, void* idk) {
    _on_exit_level();
    return onEditor(self, idk);
}

bool __fastcall Hooks::PauseLayer_init_H(gd::PauseLayer* self, int) {
    if (PauseLayer_init(self)) {
        auto win_size = CCDirector::sharedDirector()->getWinSize();
        
        auto menu = CCMenu::create();
        menu->setPosition(35, win_size.height - 40.f);
        self->addChild(menu);
        
        auto sprite = CCSprite::create("GJ_button_01.png");
        sprite->setScale(0.72f);
        auto btn = gd::CCMenuItemSpriteExtra::create(sprite, self, menu_selector(OverlayLayer::open_btn_callback));
        menu->addChild(btn);
        
        auto label = CCLabelBMFont::create("ReplayBot", "bigFont.fnt");
        label->setAnchorPoint({0, 0.5});
        label->setScale(0.5f);
        label->setPositionX(20);
        menu->addChild(label);
        return true;
    }
    return false;
}

void _handle_activated_object(bool a, bool b, gd::GameObject* object) {
    auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
    auto& rs = ReplaySystem::get_instance();
    if (play_layer && play_layer->m_isPracticeMode && rs.is_recording()) {
        if (object->m_hasBeenActivated && !a)
            rs.get_practice_fixes().add_activated_object(object);
        if (object->m_hasBeenActivatedP2 && !b)
            rs.get_practice_fixes().add_activated_object_p2(object);
    }
}

void __fastcall Hooks::PlayerObject_ringJump_H(gd::PlayerObject* self, int, gd::GameObject* ring) {
    bool a = ring->m_hasBeenActivated;
    bool b = ring->m_hasBeenActivatedP2;
    PlayerObject_ringJump(self, ring);
    _handle_activated_object(a, b, ring);
}

void __fastcall Hooks::GameObject_activateObject_H(gd::GameObject* self, int, gd::PlayerObject* player) {
    bool a = self->m_hasBeenActivated;
    bool b = self->m_hasBeenActivatedP2;
    GameObject_activateObject(self, player);
    _handle_activated_object(a, b, self);
}

void __fastcall Hooks::GJBaseGameLayer_bumpPlayer_H(gd::GJBaseGameLayer* self, int, gd::PlayerObject* player, gd::GameObject* object) {
    bool a = object->m_hasBeenActivated;
    bool b = object->m_hasBeenActivatedP2;
    GJBaseGameLayer_bumpPlayer(self, player, object);
    _handle_activated_object(a, b, object);
}

void __fastcall Hooks::PlayLayer::updateVisiblity_H(gd::PlayLayer* self, int) {
    if (!g_disable_render)
        updateVisiblity(self);
}
