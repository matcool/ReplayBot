#pragma once
#include "includes.h"
#include "utils.hpp"

class OverlayLayer : public CCLayerColor, public gd::FLAlertLayerProtocol {
protected:
    gd::CCTextInputNode* m_pIptFps;
    CCLabelBMFont* m_pLblReplay;
public:
    static OverlayLayer* create();
    bool initMyStuff();
    bool ccTouchBegan(CCTouch* touch, CCEvent* e) override {
        std::cout << "touch!" << std::endl;
        return false;
    }
    void keyBackClicked() {
        std::cout << "Hello??" << std::endl;
        CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::FadeTransition);
    }
    void _btnBackCallback(CCObject*) {
        std::cout << "Hello2??" << std::endl;
        // ???? why tf does this not work
        this->keyBackClicked();
        // i give up
        _updateDefaultFPS();
        CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::FadeTransition);
    }
    void btnCallback(CCObject*) {
        auto layer = OverlayLayer::create();
        auto scene = CCScene::create();
        scene->addChild(layer);
        CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
        // CCDirector::sharedDirector()->getRunningScene()->addChild(OverlayLayer::create());
    }
    void cbRecordBtn(CCObject*);
    void cbPlayBtn(CCObject*);
    void cbLoadBtn(CCObject*);
    void cbSaveBtn(CCObject*);

    void _updateReplayLabel();
    void _updateDefaultFPS();

    void FLAlert_Clicked(gd::FLAlertLayer* alert, bool btn2) {
        std::cout << "lol!!" << std::endl;
    }
};

class whycantijusthavethisinthelayeridontgetit : public gd::FLAlertLayerProtocol {
public:
    void FLAlert_Clicked(gd::FLAlertLayer* alert, bool btn2);
};