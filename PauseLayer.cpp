#include "PauseLayer.h"
#include "ReplaySystem.h"
#include "PlayLayer.h"

void PauseLayer::setup(uintptr_t base) {
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x1E4620),
        initHook,
        reinterpret_cast<void**>(&init)
    );
}

void __fastcall PauseLayer::initHook(CCLayer* self, void*) {
    auto director = CCDirector::sharedDirector();
    auto winSize = director->getWinSize();

    init(self);

    auto sprite = CCSprite::create("GJ_button_01.png");
    auto recordBtn = CCMenuItemSpriteExtra::create(sprite, sprite, self, menu_selector(PauseLayer::Callbacks::recordBtn));
    recordBtn->setScale(0.6f);

    auto recordLabel = CCLabelBMFont::create("Record", "bigFont.fnt");
    recordLabel->setScale(0.5f);
    recordLabel->setPositionX(15.f + recordLabel->getScaledContentSize().width / 2.f);

    auto playBtn = CCMenuItemSpriteExtra::create(sprite, sprite, self, menu_selector(PauseLayer::Callbacks::playBtn));
    playBtn->setScale(0.6f);

    auto playLabel = CCLabelBMFont::create("Play", "bigFont.fnt");
    playLabel->setScale(0.5f);
    playLabel->setPositionX(15.f + playLabel->getScaledContentSize().width / 2.f);

    playBtn->setPositionY(-30.f);
    playLabel->setPositionY(-30.f);

    auto menu = CCMenu::create();
    menu->addChild(recordBtn);
    menu->addChild(recordLabel);

    menu->addChild(playBtn);
    menu->addChild(playLabel);

    menu->setPosition(25, winSize.height - 40.f);

    self->addChild(menu);
}

void PauseLayer::Callbacks::recordBtn(CCObject*) {
    ReplaySystem::getInstance()->toggleRecording();
}

void PauseLayer::Callbacks::playBtn(CCObject*) {
    ReplaySystem::getInstance()->togglePlaying();
}