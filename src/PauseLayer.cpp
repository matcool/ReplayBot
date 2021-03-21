#include "PauseLayer.h"
#include "ReplaySystem.h"
#include "PlayLayer.h"
#include "PlayerObject.h"
#include "hook_utils.hpp"
#include "overlay.hpp"

void PauseLayer::setup(uintptr_t base) {
    Hooks::addHook(base + 0x1E4620, initHook, &init);
    Hooks::addHook(base + 0x20D3C0, onPauseHook, &onPause);
}

// this is probably on PlayLayer but i dontcare
void __fastcall PauseLayer::onPauseHook(void* self, void*, void* idk) {
    auto rs = ReplaySystem::getInstance();
    if (rs->isPlaying())
        PlayerObjectHooks::preventInput = true;
    if (rs->isRecording())
        rs->recordAction(false, true);
    onPause(self, idk);
    PlayerObjectHooks::preventInput = false;
}

bool __fastcall PauseLayer::initHook(CCLayer* self, void*) {
    auto director = CCDirector::sharedDirector();
    auto winSize = director->getWinSize();

    bool ret = init(self);

    auto sprite = CCSprite::create("GJ_button_01.png");
    auto button = CCMenuItemSpriteExtra::create(sprite, sprite, self, menu_selector(OverlayLayer::btnCallback));
    button->setScale(0.6f);

    auto label = CCLabelBMFont::create("ReplayBot", "bigFont.fnt");
    label->setScale(0.5f);
    label->setPositionX(15.f + label->getScaledContentSize().width / 2.f);

    auto menu = CCMenu::create();
    menu->addChild(button);
    menu->addChild(label);
    menu->setPosition(25, winSize.height - 40.f);

    self->addChild(menu);
    return ret;
}
