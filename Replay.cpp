#include "pch.h"
#include "Replay.h"
#include <iostream>

using namespace cocos2d;

uintptr_t follow(uintptr_t);

bool __fastcall Replay::pushButtonHook(void* self, void* _edx, void* PlayerButton) {
    auto x = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(self) + 0x67c);
 
    // no duplicate actions
    if (recording && reinterpret_cast<uintptr_t>(self) != player2) {
        actions.push_back({ *x, true });
    } else if (playing) {
        return false;
    }
    // std::cout << "Player@" << std::hex << self << " pushButton at x: " << *x << std::endl;
    return Replay::pushButton(self, PlayerButton);
}

bool __fastcall Replay::releaseButtonHook(void* self, void* _edx, void* PlayerButton) {
    auto x = reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(self) + 0x67c);
    if (recording && reinterpret_cast<uintptr_t>(self) != player2) {
        actions.push_back({ *x, false });
    } else if (playing) {
        return false;
    }
    // std::cout << "Player@" << std::hex << self << " releaseButton" << std::endl;
    return Replay::releaseButton(self, PlayerButton);
}

void __fastcall Replay::playLayerUpdateHook(cocos2d::CCLayer* self, void* _edx, float delta) {
    // std::cout << delta << std::endl;
    delta = 1.f / 120.f; // wtf this is like speedhack
    playLayer = self;

    auto label = reinterpret_cast<cocos2d::CCLabelTTF*>(self->getChildByTag(666));
    player = follow(reinterpret_cast<uintptr_t>(self) + 0x224);
    // TODO: no 2 player mode support
    player2 = follow(reinterpret_cast<uintptr_t>(self) + 0x228);
    auto x = reinterpret_cast<float*>(player + 0x67c);
    auto x2 = reinterpret_cast<float*>(player2 + 0x67c);

    if (recording) {
        if (*x < lastPlayerX) {
            std::cout << "Player died at " << lastPlayerX << std::endl;

            actions.erase(std::remove_if(actions.begin(), actions.end(), [&](Action action) -> bool {
                return action.x > *x;
            }), actions.end());
        }
        lastPlayerX = *x;
    }

    if (playing) {
        if (curPlayingAction < actions.size()) {
            auto curAction = actions[curPlayingAction];
            if (*x >= curAction.x) {
                *x = curAction.x;
                *x2 = curAction.x;
                if (curAction.hold) {
                    Replay::pushButton(reinterpret_cast<void*>(player), 0);
                    Replay::pushButton(reinterpret_cast<void*>(player2), 0);
                } else {
                    Replay::releaseButton(reinterpret_cast<void*>(player), 0);
                    Replay::releaseButton(reinterpret_cast<void*>(player2), 0);
                }
                curPlayingAction++;
            }
        } else {
            playing = false;
            label->setString("");
            std::cout << "Ran out of actions" << std::endl;
        }
    }

    return Replay::playLayerUpdate(self, delta);
}

void leftAlignLabel(CCLabelBMFont* label) {
    auto size = label->getScaledContentSize();
    auto xPos = label->getPositionX();
    label->setPositionX(xPos + size.width / 2.f);
}

void Replay::toggleRecording() {
    auto label = reinterpret_cast<cocos2d::CCLabelTTF*>(playLayer->getChildByTag(666));
    recording = !recording;
    playing = false;
    std::cout << "Recording is now " << recording << std::endl;
    if (recording) {
        lastPlayerX = 0.f;
        actions.clear();
        label->setString("Recording");
    }
    else {
        label->setString("");
    }
}

void Replay::togglePlaying() {
    auto label = reinterpret_cast<cocos2d::CCLabelTTF*>(playLayer->getChildByTag(666));
    playing = !playing;
    recording = false;
    std::cout << "Playing is now " << playing << std::endl;
    if (playing) {
        curPlayingAction = 0;
        label->setString("Playing");
    }
    else {
        label->setString("");
    }
}

// TODO: somehow call the PauseLayer::onRestart function

void Callbacks::recordBtnCallback(CCObject* psender) {
    Replay::toggleRecording();
}

void Callbacks::playBtnCallback(CCObject* psender) {
    Replay::togglePlaying();
}


void __fastcall Replay::pauseLayerInitHook(CCLayer* self, void* _edx) {
    std::cout << "PauseLayer init" << std::endl;

    auto director = cocos2d::CCDirector::sharedDirector();
    auto winSize = director->getWinSize();

    pauseLayerInit(self);

    // TODO: put this all in a CCNode so i dont have to space everything thaat manually

    auto sprite = cocos2d::CCSprite::createWithSpriteFrameName("GJ_rotationControlBtn01_001.png");
    auto btn = CCMenuItemSpriteExtra::create(sprite, sprite, self, menu_selector(Callbacks::recordBtnCallback));
    if (recording) {
        btn->setColor({ 200, 0, 0 });
    }
    btn->setPosition(winSize.width / 2.f - 100.f, winSize.height / 2.f - 70.f);
    auto label = cocos2d::CCLabelBMFont::create("Record", "bigFont.fnt");
    label->setScale(0.5f);
    label->setPosition(winSize.width - 85.f, winSize.height - 70.f);
    leftAlignLabel(label);

    auto sprite2 = cocos2d::CCSprite::createWithSpriteFrameName("GJ_rotationControlBtn01_001.png");
    auto btn2 = CCMenuItemSpriteExtra::create(sprite2, sprite2, self, menu_selector(Callbacks::playBtnCallback));
    if (playing) {
        btn2->setColor({ 200, 0, 0 });
    }
    btn2->setPosition(winSize.width / 2.f - 100.f, winSize.height / 2.f - 100.f);
    auto label2 = cocos2d::CCLabelBMFont::create("Play", "bigFont.fnt");
    label2->setScale(0.5f);
    label2->setPosition(winSize.width - 85.f, winSize.height - 100.f);
    leftAlignLabel(label2);


    auto menu = cocos2d::CCMenu::create();
    menu->addChild(btn);
    menu->addChild(btn2);
    self->addChild(menu);
    self->addChild(label);
    self->addChild(label2);
}

void __fastcall Replay::playLayerInitHook(cocos2d::CCLayer* self, void* _edx, void* GJLevel) {
    std::cout << "PlayLayer init" << std::endl;

    auto director = cocos2d::CCDirector::sharedDirector();
    auto winSize = director->getWinSize();

    playLayerInit(self, GJLevel);

    auto label = cocos2d::CCLabelTTF::create("hey", "Arial", 14);
    label->setHorizontalAlignment(cocos2d::CCTextAlignment::kCCTextAlignmentRight);
    label->setVerticalAlignment(cocos2d::CCVerticalTextAlignment::kCCVerticalTextAlignmentTop);
    label->setPosition({ winSize.width - 50.f, winSize.height - 20.f});
    label->setZOrder(999);
    label->setTag(666);

    self->addChild(label);
}

void Replay::memInit(uintptr_t base) {
    playing = false;
    recording = false;
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x1F4E40),
        Replay::pushButtonHook,
        reinterpret_cast<void**>(&Replay::pushButton)
    );
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x1F4F70),
        Replay::releaseButtonHook,
        reinterpret_cast<void**>(&Replay::releaseButton)
    );
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x2029C0),
        Replay::playLayerUpdateHook,
        reinterpret_cast<void**>(&Replay::playLayerUpdate)
    );
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x1E4620),
        Replay::pauseLayerInitHook,
        reinterpret_cast<void**>(&Replay::pauseLayerInit)
    );
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x01FB780),
        Replay::playLayerInitHook,
        reinterpret_cast<void**>(&Replay::playLayerInit)
    );
}