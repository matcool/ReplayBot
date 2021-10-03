#include "hooks.hpp"
#include "replay_system.hpp"
#include "overlay_layer.hpp"
#include "recorder.hpp"
#include <chrono>
#include <matdash/minhook.hpp>

auto cocos(const char* symbol) {
    static auto mod = GetModuleHandleA("libcocos2d.dll");
    return GetProcAddress(mod, symbol);
}

void Hooks::init() {
    add_hook<&CCScheduler_update, Thiscall>(cocos("?update@CCScheduler@cocos2d@@UAEXM@Z"));
    add_hook<&CCKeyboardDispatcher_dispatchKeyboardMSG>(cocos("?dispatchKeyboardMSG@CCKeyboardDispatcher@cocos2d@@QAE_NW4enumKeyCodes@2@_N@Z"));
    add_hook<&CheckpointObject_create, Optfastcall>(gd::base + 0x20ddd0);

    add_hook<&PlayLayer::update, Thiscall>(gd::base + 0x2029C0);

    add_hook<&PlayLayer::pushButton>(gd::base + 0x111500);
    add_hook<&PlayLayer::releaseButton>(gd::base + 0x111660);
    add_hook<&PlayLayer::resetLevel>(gd::base + 0x20BF00);

    add_hook<&PlayLayer::pauseGame>(gd::base + 0x20D3C0);

    add_hook<&PlayLayer::levelComplete>(gd::base + 0x1FD3D0);
    add_hook<&PlayLayer::onQuit>(gd::base + 0x20D810);
    add_hook<&PauseLayer_onEditor>(gd::base + 0x1E60E0);

    add_hook<&PlayLayer::updateVisiblity>(gd::base + 0x205460);

    add_hook<&PauseLayer_init>(gd::base + 0x1E4620);

    add_hook<&PlayerObject_ringJump>(gd::base + 0x1f4ff0);
    add_hook<&GameObject_activateObject>(gd::base + 0xef0e0);
    add_hook<&GJBaseGameLayer_bumpPlayer>(gd::base + 0x10ed50);
}

// yes these are global, too lazy to store them in replaysystem or smth
// not like theyre used anywhere else atm
bool g_disable_render = false;
float g_left_over = 0.f;

void Hooks::CCScheduler_update(CCScheduler* self, float dt) {
    auto& rs = ReplaySystem::get_instance();
    if (rs.recorder.m_recording || rs.is_playing() || rs.is_recording() && gd::GameManager::sharedState()->getPlayLayer()) {
        const auto fps = rs.get_replay().get_fps();
        auto speedhack = self->getTimeScale();

        const float target_dt = 1.f / fps / speedhack;

        if (!rs.real_time_mode)
            return orig<&CCScheduler_update, Thiscall>(self, target_dt);

        // todo: find ways to disable more render stuff
        g_disable_render = false;

        unsigned times = static_cast<int>((dt + g_left_over) / target_dt);
        if (dt == 0.f)
            return orig<&CCScheduler_update, Thiscall>(self, target_dt);
        auto start = std::chrono::high_resolution_clock::now();
        for (unsigned i = 0; i < times; ++i) {
            // if (i == times - 1)
            //     g_disable_render = false;
            orig<&CCScheduler_update, Thiscall>(self, target_dt);
            using namespace std::literals;
            if (std::chrono::high_resolution_clock::now() - start > 33.333ms) {
                times = i + 1;
                break;
            }
        }
        g_left_over += dt - target_dt * times;
    } else {
        orig<&CCScheduler_update, Thiscall>(self, dt);
    }
}

void Hooks::CCKeyboardDispatcher_dispatchKeyboardMSG(CCKeyboardDispatcher* self, int key, bool down) {
    auto& rs = ReplaySystem::get_instance();
    if (down) {
        auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
        if (rs.is_recording() && play_layer) {
            if (key == 'C') {
                rs.set_frame_advance(false);
                PlayLayer::update(play_layer, 1.f / rs.get_default_fps());
                rs.set_frame_advance(true);
            } else if (key == 'F') {
                rs.set_frame_advance(false);
            } else if (key == 'R') {
                PlayLayer::resetLevel(play_layer);
            }
        }
    }
    orig<&CCKeyboardDispatcher_dispatchKeyboardMSG>(self, key, down);
}

float g_xpos_time = 0.f;

void Hooks::PlayLayer::update(gd::PlayLayer* self, float dt) {
    auto& rs = ReplaySystem::get_instance();
    if (rs.get_frame_advance()) return;
    if (rs.is_playing()) rs.handle_playing();
    if (rs.recorder.m_recording) {
        rs.recorder.handle_recording(self, dt);
    }
    orig<&update, Thiscall>(self, dt);
}


bool _player_button_handler(bool hold, bool button) {
    if (gd::GameManager::sharedState()->getPlayLayer()) {
        auto& rs = ReplaySystem::get_instance();
        if (rs.is_playing()) return true;
        rs.record_action(hold, button);
    }
    return false;
}

void Hooks::PlayLayer::pushButton(gd::PlayLayer* self, int idk, bool button) {
    if (_player_button_handler(true, button)) return;
    orig<&pushButton>(self, idk, button);
}

void Hooks::PlayLayer::releaseButton(gd::PlayLayer* self, int idk, bool button) {
    if (_player_button_handler(false, button)) return;
    orig<&releaseButton>(self, idk, button);
}

void Hooks::PlayLayer::resetLevel(gd::PlayLayer* self) {
    orig<&resetLevel>(self);
    auto& rs = ReplaySystem::get_instance();
    rs.on_reset();
    // from what i've checked rob doesnt store this anywhere, so i have to calculate it again
    if (rs.recorder.m_recording)
        rs.recorder.m_song_start_offset = self->timeForXPos2(
            self->m_player1->m_position.x, self->m_isTestMode) + self->m_levelSettings->m_songStartOffset;
}


void Hooks::PlayLayer::pauseGame(gd::PlayLayer* self, bool idk) {
    auto addr = cast<void*>(gd::base + 0x20D43C);
    auto& rs = ReplaySystem::get_instance();
    if (rs.is_recording())
        rs.record_action(false, true, false);

    bool should_patch = rs.is_playing();
    if (should_patch)
        patch(addr, {0x83, 0xC4, 0x04, 0x90, 0x90});
    
    orig<&pauseGame>(self, idk);

    if (should_patch)
        patch(addr, {0xe8, 0x2f, 0x7b, 0xfe, 0xff});
}


CCObject* Hooks::CheckpointObject_create() {
    return CheckpointObjectMod::create();
}

void Hooks::PlayLayer::levelComplete(gd::PlayLayer* self) {
    ReplaySystem::get_instance().reset_state();
    return orig<&levelComplete>(self);
}

void _on_exit_level() {
    auto& rs = ReplaySystem::get_instance();
    rs.reset_state();
}

void Hooks::PlayLayer::onQuit(gd::PlayLayer* self) {
    _on_exit_level();
    orig<&onQuit>(self);
}

void Hooks::PauseLayer_onEditor(gd::PauseLayer* self, CCObject* idk) {
    _on_exit_level();
    orig<&PauseLayer_onEditor>(self, idk);
}

bool Hooks::PauseLayer_init(gd::PauseLayer* self) {
    if (orig<&PauseLayer_init>(self)) {
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

void Hooks::PlayerObject_ringJump(gd::PlayerObject* self, gd::GameObject* ring) {
    bool a = ring->m_hasBeenActivated;
    bool b = ring->m_hasBeenActivatedP2;
    orig<&PlayerObject_ringJump>(self, ring);
    _handle_activated_object(a, b, ring);
}

void Hooks::GameObject_activateObject(gd::GameObject* self, gd::PlayerObject* player) {
    bool a = self->m_hasBeenActivated;
    bool b = self->m_hasBeenActivatedP2;
    orig<&GameObject_activateObject>(self, player);
    _handle_activated_object(a, b, self);
}

void Hooks::GJBaseGameLayer_bumpPlayer(gd::GJBaseGameLayer* self, gd::PlayerObject* player, gd::GameObject* object) {
    bool a = object->m_hasBeenActivated;
    bool b = object->m_hasBeenActivatedP2;
    orig<&GJBaseGameLayer_bumpPlayer>(self, player, object);
    _handle_activated_object(a, b, object);
}

void Hooks::PlayLayer::updateVisiblity(gd::PlayLayer* self) {
    if (!g_disable_render)
        orig<&updateVisiblity>(self);
}
