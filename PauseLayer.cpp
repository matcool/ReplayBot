#include "PauseLayer.h"
#include "ReplaySystem.h"
#include "PlayLayer.h"
#include "PlayerObject.h"

void PauseLayer::setup(uintptr_t base) {
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x1E4620),
        initHook,
        reinterpret_cast<void**>(&init)
    );
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x20D3C0),
        onPauseHook,
        reinterpret_cast<void**>(&onPause)
    );
}

void PauseLayer::unload(uintptr_t base) {
    MH_RemoveHook(reinterpret_cast<void*>(base + 0x1E4620));
    MH_RemoveHook(reinterpret_cast<void*>(base + 0x20D3C0));
}

// this is probably on PlayLayer but i dontcare
void __fastcall PauseLayer::onPauseHook(void* self, void*, void* idk) {
    auto rs = ReplaySystem::getInstance();
    if (rs->isPlaying())
        PlayerObject::preventInput = true;
    if (rs->isRecording())
        rs->recordAction(false, true);
    onPause(self, idk);
    PlayerObject::preventInput = false;
}

void __fastcall PauseLayer::initHook(CCLayer* self, void*) {
    auto director = CCDirector::sharedDirector();
    auto winSize = director->getWinSize();

    init(self);

    auto sprite = CCSprite::create("GJ_button_01.png");
    auto sprite2 = CCSprite::create("GJ_button_02.png");
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

    auto loadBtn = CCMenuItemSpriteExtra::create(sprite2, sprite2, self, menu_selector(PauseLayer::Callbacks::loadBtn));
    loadBtn->setScale(0.6f);

    auto loadLabel = CCLabelBMFont::create("Load", "bigFont.fnt");
    loadLabel->setScale(0.5f);
    loadLabel->setPositionX(15.f + loadLabel->getScaledContentSize().width / 2.f);

    loadBtn->setPositionY(-80.f);
    loadLabel->setPositionY(-80.f);

    auto saveBtn = CCMenuItemSpriteExtra::create(sprite2, sprite2, self, menu_selector(PauseLayer::Callbacks::saveBtn));
    saveBtn->setScale(0.6f);

    auto saveLabel = CCLabelBMFont::create("Save", "bigFont.fnt");
    saveLabel->setScale(0.5f);
    saveLabel->setPositionX(15.f + saveLabel->getScaledContentSize().width / 2.f);

    saveBtn->setPositionY(-110.f);
    saveLabel->setPositionY(-110.f);

    auto menu = CCMenu::create();
    menu->addChild(recordBtn);
    menu->addChild(recordLabel);

    menu->addChild(playBtn);
    menu->addChild(playLabel);

    menu->addChild(loadBtn);
    menu->addChild(loadLabel);

    menu->addChild(saveBtn);
    menu->addChild(saveLabel);

    menu->setPosition(25, winSize.height - 40.f);

    self->addChild(menu);
}

void PauseLayer::Callbacks::recordBtn(CCObject*) {
    ReplaySystem::getInstance()->toggleRecording();
}

void PauseLayer::Callbacks::playBtn(CCObject*) {
    ReplaySystem::getInstance()->togglePlaying();
}

void PauseLayer::Callbacks::loadBtn(CCObject*) {
    OPENFILENAMEA info;
    ZeroMemory(&info, sizeof info);
    CHAR fileName[MAX_PATH] = "";
    info.lStructSize = sizeof info;
    info.hwndOwner = NULL;
    info.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
    info.lpstrFile = fileName;
    info.nMaxFile = MAX_PATH;
    if (GetOpenFileNameA(&info)) {
        std::cout << "Loading file: " << info.lpstrFile << std::endl;
        ReplaySystem::getInstance()->loadReplay(info.lpstrFile);
    }
}

void PauseLayer::Callbacks::saveBtn(CCObject*) {
    OPENFILENAMEA info;
    ZeroMemory(&info, sizeof info);
    CHAR fileName[MAX_PATH] = "";
    info.lStructSize = sizeof info;
    info.hwndOwner = NULL;
    info.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    info.lpstrFile = fileName;
    info.nMaxFile = MAX_PATH;
    info.lpstrDefExt = "replay";
    if (GetSaveFileNameA(&info)) {
        std::cout << "Saving file: " << info.lpstrFile << std::endl;
        ReplaySystem::getInstance()->saveReplay(info.lpstrFile);
    }
}