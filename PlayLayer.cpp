#include "PlayLayer.h"
#include "ReplaySystem.h"
#include "PlayerObject.h"
#include "utils.hpp"

void PlayLayer::setup(uintptr_t base) {
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x01FB780),
        PlayLayer::initHook,
        reinterpret_cast<void**>(&PlayLayer::init)
    );
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x2029C0),
        PlayLayer::updateHook,
        reinterpret_cast<void**>(&PlayLayer::update)
    );
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x1FD3D0),
        PlayLayer::levelCompleteHook,
        reinterpret_cast<void**>(&PlayLayer::levelComplete)
    );
}

void PlayLayer::unload(uintptr_t base) {
    MH_RemoveHook(reinterpret_cast<void*>(base + 0x01FB780));
    MH_RemoveHook(reinterpret_cast<void*>(base + 0x2029C0));
    MH_RemoveHook(reinterpret_cast<void*>(base + 0x1FD3D0));
}

void __fastcall PlayLayer::initHook(CCLayer* self, void*, void* GJLevel) {
    PlayLayer::self = self;
    init(self, GJLevel);

    // maybe switch to label atlas or whatever
    auto label = CCLabelTTF::create("", "Arial", 14);
    label->setZOrder(999);
    label->setTag(StatusLabelTag);

    self->addChild(label);
}

void PlayLayer::updateStatusLabel(const char* text) {
    auto director = CCDirector::sharedDirector();
    auto winSize = director->getWinSize();

    auto label = reinterpret_cast<cocos2d::CCLabelTTF*>(self->getChildByTag(StatusLabelTag));
    label->setString(text);
    label->setPosition({ 10.f + label->getContentSize().width / 2.f, 10.f });
}

void __fastcall PlayLayer::updateHook(CCLayer* self, void*, float dt) {
    auto rs = ReplaySystem::getInstance();
    if (rs->isPlaying() || rs->isRecording()) {
        dt = 1.f / rs->getCurrentReplay()->getFPS();
    }
    if (rs->isPlaying()) {
        rs->handlePlaying();
    }
    if (rs->isRecording()) {
        rs->handleRecording();
    }
    update(self, dt);
}

void* __fastcall PlayLayer::levelCompleteHook(CCLayer* self, void*) {
    auto rs = ReplaySystem::getInstance();
    if (rs->isRecording()) {
        std::cout << "Level completed, stopping recording" << std::endl;
        rs->toggleRecording();
    }
    return levelComplete(self);
}

uintptr_t PlayLayer::getPlayer() {
    if (self) return follow(reinterpret_cast<uintptr_t>(self) + 0x224);
    return 0;
}
uintptr_t PlayLayer::getPlayer2() {
    if (self) return follow(reinterpret_cast<uintptr_t>(self) + 0x228);
    return 0;
}