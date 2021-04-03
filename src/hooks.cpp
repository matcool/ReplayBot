#include "hooks.hpp"
#include "replay_system.hpp"

auto add_gd_hook(uintptr_t offset, void* hook, void* orig) {
    return MH_CreateHook(cast<void*>(cast<uintptr_t>(gd::base) + offset), hook, cast<void**>(orig));
}
#define ADD_GD_HOOK(o, f) add_gd_hook(o, f##_H, &f)

auto add_cocos_hook(const char* symbol, void* hook, void* orig) {
    return MH_CreateHook(GetProcAddress(GetModuleHandleA("libcocos2d.dll"), symbol), hook, cast<void**>(orig));
}
#define ADD_COCOS_HOOK(o, f) add_cocos_hook(o, f##_H, &f)

void Hooks::init() {
    ADD_COCOS_HOOK("?update@CCScheduler@cocos2d@@UAEXM@Z", CCScheduler_update);

    ADD_GD_HOOK(0x1FB780, _PlayLayer::init);
    ADD_GD_HOOK(0x2029C0, _PlayLayer::update);

    ADD_GD_HOOK(0x111500, _PlayLayer::pushButton);
    ADD_GD_HOOK(0x111660, _PlayLayer::releaseButton);
    ADD_GD_HOOK(0x20BF00, _PlayLayer::resetLevel);

    ADD_GD_HOOK(0x20D3C0, _PlayLayer::pauseGame);

    ADD_GD_HOOK(0x20B050, _PlayLayer::createCheckpoint);
    ADD_GD_HOOK(0x20B830, _PlayLayer::removeLastCheckpoint);

    ADD_GD_HOOK(0x1FD3D0, _PlayLayer::levelComplete);
    ADD_GD_HOOK(0x20D810, _PlayLayer::onQuit);
    ADD_GD_HOOK(0x1E60E0, _PlayLayer::onEditor);

    ADD_GD_HOOK(0x1E4620, PauseLayer_init);
}

void __fastcall Hooks::CCScheduler_update_H(CCScheduler* self, int, float dt) {
    auto rs = ReplaySystem::get_instance();
    if (rs->is_playing() || rs->is_recording() && gd::GameManager::sharedState()->getPlayLayer()) {
        auto fps = rs->get_replay().get_fps();
        auto speedhack = self->getTimeScale();
        dt = 1.f / fps / speedhack;
    }
    CCScheduler_update(self, dt);
}


bool __fastcall Hooks::_PlayLayer::init_H(PlayLayer* self, int, void* level) {
    return init(self, level);
}

void __fastcall Hooks::_PlayLayer::update_H(PlayLayer* self, int, float dt) {
    auto rs = ReplaySystem::get_instance();
    if (rs->is_playing())
        rs->handle_playing();
    update(self, dt);
}


bool _player_button_handler(bool hold, bool button) {
    if (gd::GameManager::sharedState()->getPlayLayer()) {
        auto rs = ReplaySystem::get_instance();
        if (rs->is_playing()) return true;
        rs->record_action(hold, button);
        rs->get_practice_fixes().set_holding(hold);
    }
    return false;
}

int __fastcall Hooks::_PlayLayer::pushButton_H(PlayLayer* self, int, int idk, bool button) {
    if (_player_button_handler(true, button)) return 0;
    return pushButton(self, idk, button);
}

int __fastcall Hooks::_PlayLayer::releaseButton_H(PlayLayer* self, int, int idk, bool button) {
    if (_player_button_handler(false, button)) return 0;
    return releaseButton(self, idk, button);
}

int __fastcall Hooks::_PlayLayer::resetLevel_H(PlayLayer* self, int) {
    auto ret = resetLevel(self);
    ReplaySystem::get_instance()->on_reset();
    return ret;
}


void __fastcall Hooks::_PlayLayer::pauseGame_H(PlayLayer* self, int, bool idk) {
    auto addr = cast<void*>(gd::base + 0x20D43C);
    auto rs = ReplaySystem::get_instance();
    if (rs->is_recording())
        rs->record_action(false, true, false);

    bool should_patch = rs->is_playing();
    if (should_patch)
        patch(addr, {0x83, 0xC4, 0x04, 0x90, 0x90});
    
    pauseGame(self, idk);

    if (should_patch)
        patch(addr, {0xe8, 0x2f, 0x7b, 0xfe, 0xff});
}


int __fastcall Hooks::_PlayLayer::createCheckpoint_H(PlayLayer* self, int) {
    auto rs = ReplaySystem::get_instance();
    if (rs->is_recording()) rs->get_practice_fixes().add_checkpoint();
    return createCheckpoint(self);
}

void* __fastcall Hooks::_PlayLayer::removeLastCheckpoint_H(PlayLayer* self, int) {
    ReplaySystem::get_instance()->get_practice_fixes().remove_checkpoint();
    return removeLastCheckpoint(self);
}


void* __fastcall Hooks::_PlayLayer::levelComplete_H(PlayLayer* self, int) {
    ReplaySystem::get_instance()->reset_state();
    return levelComplete(self);
}

void _on_exit_level() {
    auto rs = ReplaySystem::get_instance();
    rs->get_practice_fixes().clear_checkpoints();
    rs->reset_state();
}

void* __fastcall Hooks::_PlayLayer::onQuit_H(PlayLayer* self, int) {
    _on_exit_level();
    return onQuit(self);
}

void* __fastcall Hooks::_PlayLayer::onEditor_H(PlayLayer* self, int, void* idk) {
    _on_exit_level();
    return onEditor(self, idk);
}

class penisdick {
    public:
    void thecallback(CCObject*) {
        auto rs = ReplaySystem::get_instance();
        // amazing
        if (cast<int>(this) == 1) {
            rs->toggle_recording();
        } else if (cast<int>(this) == 2) {
            rs->toggle_playing();
        } else if (cast<int>(this) == 3) {
            rs->get_replay().save("zzzzjiodsjoisajd.replay");
        } else if (cast<int>(this) == 4) {
            rs->get_replay() = Replay::load("zzzzjiodsjoisajd.replay");
        }
    }
};

bool __fastcall Hooks::PauseLayer_init_H(gd::PauseLayer* self, int) {
    if (PauseLayer_init(self)) {
        auto menu = CCMenu::create();
        auto sprite = CCSprite::create("GJ_button_01.png");
        auto btn1 = gd::CCMenuItemSpriteExtra::create(sprite, cast<CCObject*>(1), menu_selector(penisdick::thecallback));
        auto btn2 = gd::CCMenuItemSpriteExtra::create(sprite, cast<CCObject*>(2), menu_selector(penisdick::thecallback));
        btn2->setPosition({50, 50});
        auto btn3 = gd::CCMenuItemSpriteExtra::create(sprite, cast<CCObject*>(3), menu_selector(penisdick::thecallback));
        btn3->setPosition({70, 70});
        auto btn4 = gd::CCMenuItemSpriteExtra::create(sprite, cast<CCObject*>(4), menu_selector(penisdick::thecallback));
        btn4->setPosition({90, 90});
        menu->addChild(btn1);
        menu->addChild(btn2);
        menu->addChild(btn3);
        menu->addChild(btn4);
        self->addChild(menu);
        return true;
    }
    return false;
}