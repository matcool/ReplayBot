#include "PlayLayer.h"
#include "ReplaySystem.h"
#include "utils.hpp"
#include "PracticeFixes.h"

void PlayLayer::setup(uintptr_t base) {
    PlayLayer::base = base;
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x1FB780),
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
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x20D810),
        PlayLayer::onQuitHook,
        reinterpret_cast<void**>(&PlayLayer::onQuit)
    );
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x111500),
        pushButtonHook,
        reinterpret_cast<void**>(&pushButton)
    );
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x111660),
        releaseButtonHook,
        reinterpret_cast<void**>(&releaseButton)
    );

    auto handle = GetModuleHandleA("libcocos2d.dll");
    auto address = GetProcAddress(handle, "?update@CCScheduler@cocos2d@@UAEXM@Z");
    schUpdateAddress = reinterpret_cast<void*>(address);

    MH_CreateHook(
        schUpdateAddress,
        schUpdateHook,
        reinterpret_cast<void**>(&schUpdate)
    );

    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x25FB60),
        markCheckpointHook,
        reinterpret_cast<void**>(&markCheckpoint)
    );
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x20B830),
        removeLastCheckpointHook,
        reinterpret_cast<void**>(&removeLastCheckpoint)
    );
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x1E60E0),
        onEditorHook,
        reinterpret_cast<void**>(&onEditor)
    );
}

void PlayLayer::unload(uintptr_t base) {
    MH_RemoveHook(reinterpret_cast<void*>(base + 0x1FB780));
    MH_RemoveHook(reinterpret_cast<void*>(base + 0x2029C0));
    MH_RemoveHook(reinterpret_cast<void*>(base + 0x1FD3D0));
    MH_RemoveHook(reinterpret_cast<void*>(base + 0x20D810));
    MH_RemoveHook(reinterpret_cast<void*>(base + 0x111500));
    MH_RemoveHook(reinterpret_cast<void*>(base + 0x111660));
    MH_RemoveHook(schUpdateAddress);
    MH_RemoveHook(reinterpret_cast<void*>(base + 0x25FB60));
    MH_RemoveHook(reinterpret_cast<void*>(base + 0x20B830));
    MH_RemoveHook(reinterpret_cast<void*>(base + 0x1E60E0));
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

void __fastcall PlayLayer::schUpdateHook(CCScheduler* self, void*, float dt) {
    auto rs = ReplaySystem::getInstance();
    if (rs->isPlaying() || rs->isRecording()) {
        float fps = rs->getCurrentReplay()->getFPS();
        float speedhack = self->getTimeScale();
        dt = 1.f / fps / speedhack;
    }
    return schUpdate(self, dt);
}

void __fastcall PlayLayer::updateHook(CCLayer* self, void*, float dt) {
    auto rs = ReplaySystem::getInstance();
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

void exitLevel() {
    PracticeFixes::clearCheckpoints();
    auto rs = ReplaySystem::getInstance();
    if (rs->isRecording()) {
        std::cout << "Exited out of level, stopping recording" << std::endl;
        rs->toggleRecording();
    }
    if (rs->isPlaying()) {
        std::cout << "Exited out of level, stopped playing" << std::endl;
        rs->togglePlaying();
    }
}

void* __fastcall PlayLayer::onQuitHook(CCLayer* self, void*) {
    exitLevel();
    return onQuit(self);
}

void* __fastcall PlayLayer::onEditorHook(CCLayer* self, void*, void* idk) {
    exitLevel();
    return onEditor(self, idk);
}

uint32_t __fastcall PlayLayer::pushButtonHook(CCLayer* self, void*, int idk, bool button) {
    auto rs = ReplaySystem::getInstance();
    if (rs->isPlaying()) return 0;
    rs->recordAction(true, button);
    PracticeFixes::isHolding = true;
    return pushButton(self, idk, button);
}

uint32_t __fastcall PlayLayer::releaseButtonHook(CCLayer* self, void*, int idk, bool button) {
    auto rs = ReplaySystem::getInstance();
    if (rs->isPlaying()) return 0;
    rs->recordAction(false, button);
    PracticeFixes::isHolding = false;
    return releaseButton(self, idk, button);
}

// technically not in playlayer but who cares
bool PlayLayer::is2Player() {
    return *reinterpret_cast<bool*>(follow(follow(follow(base + 0x3222D0) + 0x164) + 0x22c) + 0xfa);
}

uintptr_t PlayLayer::getPlayer() {
    if (self) return follow(reinterpret_cast<uintptr_t>(self) + 0x224);
    return 0;
}
uintptr_t PlayLayer::getPlayer2() {
    if (self) return follow(reinterpret_cast<uintptr_t>(self) + 0x228);
    return 0;
}

void* __fastcall PlayLayer::markCheckpointHook(CCLayer* self, void*, void* idk2) {
    auto isDead = reinterpret_cast<bool*>(reinterpret_cast<uintptr_t>(PlayLayer::self) + 0x39C);
    if (!*isDead) {
        PracticeFixes::addCheckpoint();
    }
    return markCheckpoint(self, idk2);
}

void* __fastcall PlayLayer::removeLastCheckpointHook(CCLayer* self, void*) {
    PracticeFixes::removeCheckpoint();
    return removeLastCheckpoint(self);
}
