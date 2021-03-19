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
    void keyBackClicked() {
        CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::FadeTransition);
    }
    void _btnBackCallback(CCObject*) {
        // ???? why tf does this not work
        // this->keyBackClicked();
        // i give up
        _updateDefaultFPS();
        // CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::FadeTransition);
        auto pauseLayer = this->getParent();
        this->removeFromParentAndCleanup(true);
        auto children = pauseLayer->getChildren();
        for (unsigned i = 1; i < pauseLayer->getChildrenCount(); ++i) {
            cast<CCNode*>(children->objectAtIndex(i))->setVisible(true);
        }
    }
    void btnCallback(CCObject*) {
        auto layer = OverlayLayer::create();
        // auto scene = CCScene::create();
        // scene->addChild(layer);
        // CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
        auto pauseLayer = cast<CCLayer*>(CCDirector::sharedDirector()->getRunningScene()->getChildren()->objectAtIndex(1));
        auto children = pauseLayer->getChildren();
        for (unsigned i = 1; i < pauseLayer->getChildrenCount(); ++i) {
            cast<CCNode*>(children->objectAtIndex(i))->setVisible(false);
        }
        pauseLayer->addChild(layer);
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