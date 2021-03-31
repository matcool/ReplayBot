#include "overlay.hpp"
#include "ReplaySystem.h"
#include <nfd.h>

OverlayLayer* OverlayLayer::create() {
    auto self = cast<OverlayLayer*>(CCLayerColor::create());
    // isnt working :sob:
    self->setKeypadEnabled(true);
    self->initMyStuff();

    return self;
}

void buttonWithLabel(CCMenu* menu, float x, float y, CCSprite* sprite, const char* text, CCObject* target, SEL_MenuHandler handler, bool right = false) {
    auto label = CCLabelBMFont::create(text, "bigFont.fnt");
    label->setScale(0.7f);
    auto button = gd::CCMenuItemSpriteExtra::create(sprite, target, handler);
    auto btnSize = button->getScaledContentSize();
    auto lblSize = label->getScaledContentSize();
    button->setPosition({x, y});
    float lbX = btnSize.width / 2.f + 5.f + lblSize.width / 2.f + x;
    label->setPosition({lbX, y});
    if (right)
        label->setPositionX(-lbX);
    menu->addChild(button);
    menu->addChild(label);
}

void OverlayLayer::_updateReplayLabel() {
    auto label = m_pLblReplay;
    auto replay = ReplaySystem::getInstance()->getCurrentReplay();
    auto winSize = CCDirector::sharedDirector()->getWinSize();
    std::stringstream stream;
    if (replay) {
        stream << "Current FPS: " << replay->getFPS();
    } else {
        stream << "No replay loaded";
    }
    label->setString(stream.str().c_str());
    auto size = label->getScaledContentSize();
    label->setPosition({15 + size.width / 2, winSize.height - 180 - size.height / 2});
}

bool OverlayLayer::initMyStuff() {
    auto director = CCDirector::sharedDirector();
    auto winSize = director->getWinSize();
    auto rs = ReplaySystem::getInstance();

    initWithColor({0, 0, 0, 0}, winSize.width, winSize.height);
    setZOrder(999);
    {
        auto btnBack = gd::CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png"),
            this,
            menu_selector(OverlayLayer::_btnBackCallback)
        );
        auto menu = CCMenu::create();
        menu->addChild(btnBack);
        menu->setPosition({25, winSize.height - 25});
        addChild(menu);
    }

    auto menu = CCMenu::create();
    auto sprGreen = CCSprite::create("GJ_button_01.png");
    sprGreen->setScale(0.72f);
    menu->setPosition({30, winSize.height - 80});
    buttonWithLabel(menu, 0, 0, sprGreen, "Record", this, menu_selector(OverlayLayer::cbRecordBtn));
    buttonWithLabel(menu, 0, -38, sprGreen, "Play", this, menu_selector(OverlayLayer::cbPlayBtn));

    addChild(menu);

    {
        auto menu = CCMenu::create();
        menu->setPosition({winSize.width - 30, winSize.height - 80});
        // auto sprFolder = CCSprite::createWithSpriteFrameName("accountBtn_myLevels_001.png");
        // sprFolder->setScale(0.77f);
        auto sprite = CCSprite::create("GJ_button_02.png");
        sprite->setScale(0.72f);
        buttonWithLabel(menu, 0, 0, sprite, "Load", this, menu_selector(OverlayLayer::cbLoadBtn), true);
        buttonWithLabel(menu, 0, -38, sprite, "Save", this, menu_selector(OverlayLayer::cbSaveBtn), true);
        addChild(menu);
    }

    {
        auto sprite = CCSprite::create("square02b_001.png");
        sprite->setColor({0, 0, 0});
        sprite->setOpacity(69);
        sprite->setPosition({102, winSize.height - 159});
        sprite->setScaleX(0.825f);
        sprite->setScaleY(0.325f);
        addChild(sprite);
    }

    auto iptFps = gd::CCTextInputNode::create("the fps uwu", this, "bigFont.fnt", 200.f, 50.f);
    iptFps->setPosition({61, winSize.height - 168});
    iptFps->setScale(0.6f);
    iptFps->setLabelPlaceholerScale(0.5f);
    iptFps->setLabelPlaceholderColor({200, 200, 200});
    iptFps->setAllowedChars("0123456789");
    iptFps->setMaxLabelLength(10); // who needs this much
    iptFps->setString(std::to_string((int)rs->getDefaultFPS()).c_str());
    addChild(iptFps);
    m_pIptFps = iptFps;

    {
        auto label = CCLabelBMFont::create("FPS:", "bigFont.fnt");
        label->setPosition({42, winSize.height - 158});
        label->setScale(0.7f);
        addChild(label);
    }

    {
        auto label = CCLabelBMFont::create("", "bigFont.fnt");
        m_pLblReplay = label;
        label->setScale(0.7f);
        _updateReplayLabel();
        addChild(label);
    }

    return true;
}

void OverlayLayer::_updateDefaultFPS() {
    std::string tmp(this->m_pIptFps->getString());
    ReplaySystem::getInstance()->setDefaultFPS((float)std::stoi(tmp));
}

void _loadReplayDialog() {
    nfdchar_t* path = nullptr;
    auto result = NFD_OpenDialog("replay", nullptr, &path);
    if (result == NFD_OKAY) {
        ReplaySystem::getInstance()->loadReplay(path);
        gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Replay loaded.")->show();
        free(path);
    }
}

void whycantijusthavethisinthelayeridontgetit::FLAlert_Clicked(gd::FLAlertLayer* alert, bool btn2) {
    auto rs = ReplaySystem::getInstance();
    if (!btn2) {
        auto i = alert->getTag();
        auto layer = cast<OverlayLayer*>(alert->getUserData()); // god this is dumb
        if (i == -1) return;
        else if (i == 1) {
            rs->toggleRecording();
            layer->_updateReplayLabel();
        } else if (i == 2) {
            _loadReplayDialog();
            layer->_updateReplayLabel();
        }
    }
}

void OverlayLayer::cbPlayBtn(CCObject*) {
    ReplaySystem::getInstance()->togglePlaying();
}

void OverlayLayer::cbRecordBtn(CCObject*) {
    _updateDefaultFPS();
    auto rs = ReplaySystem::getInstance();
    if (!rs->isRecording() && rs->getCurrentReplay()) {
        auto alert = gd::FLAlertLayer::create(
            new whycantijusthavethisinthelayeridontgetit(),
            "Warning",
            "Ok",
            "Cancel",
            "This will <cr>overwrite</c> your currently loaded replay."
        );
        alert->setUserData(this);
        alert->setTag(1);
        alert->show();
    } else {
        rs->toggleRecording();
        _updateReplayLabel();
    }
}

void OverlayLayer::cbLoadBtn(CCObject*) {
    auto rs = ReplaySystem::getInstance();
    if (rs->getCurrentReplay()) {
        auto alert = gd::FLAlertLayer::create(
            new whycantijusthavethisinthelayeridontgetit(),
            "Warning",
            "Ok",
            "Cancel",
            "This will <cr>overwrite</c> your currently loaded replay."
        );
        alert->setUserData(this);
        alert->setTag(2);
        alert->show();
    } else {
        _loadReplayDialog();
        _updateReplayLabel();
    }
}

void OverlayLayer::cbSaveBtn(CCObject*) {
    auto rs = ReplaySystem::getInstance();
    nfdchar_t* path = nullptr;
    auto result = NFD_SaveDialog("replay", nullptr, &path);
    if (result == NFD_OKAY) {
        rs->saveReplay(path);
        gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Replay saved.")->show();
        free(path);
    }
}
