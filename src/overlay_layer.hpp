#pragma once
#include "includes.h"

class OverlayLayer : public gd::FLAlertLayer {
    gd::CCTextInputNode* m_fps_input;
    CCLabelBMFont* m_replay_info;

public:
    static auto create() {
        auto node = new OverlayLayer;
        if (node && node->init()) {
            node->autorelease();
        } else {
            CC_SAFE_DELETE(node);
        }
        return node;
    }

    virtual bool init();

    void open_btn_callback(CCObject*) {
        auto node = create();
        CCDirector::sharedDirector()->getRunningScene()->addChild(node);
    }

    void close_btn_callback(CCObject*) {
        this->setKeyboardEnabled(false);
        this->removeFromParentAndCleanup(true);
    }

    // maybe do like on_btn_record idk
    void on_record(CCObject*);
    void on_play(CCObject*);
    void on_save(CCObject*);
    void on_load(CCObject*);
};