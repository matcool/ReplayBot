#pragma once
#include "includes.h"

class OverlayLayer : public gd::FLAlertLayer, public CCTextFieldDelegate, public gd::FLAlertLayerProtocol {
    gd::CCTextInputNode* m_fps_input;
    CCLabelBMFont* m_replay_info;
    gd::CCMenuItemToggler* m_x_pos_toggle;
    gd::CCMenuItemToggler* m_frame_toggle;

    inline void _update_default_fps();
    void _handle_load_replay();
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

    void update_info_text();

    void open_btn_callback(CCObject*) {
        auto node = create();
        CCDirector::sharedDirector()->getRunningScene()->addChild(node);
    }

    void close_btn_callback(CCObject*) {
        keyBackClicked();
    }

    // maybe do like on_btn_record idk
    void on_record(CCObject*);
    void on_play(CCObject*);
    void on_save(CCObject*);
    void on_load(CCObject*);

    void on_x_pos(CCObject*);
    void on_frame(CCObject*);

    void on_toggle_real_time(CCObject*);

    virtual void keyBackClicked();
    virtual void keyDown(enumKeyCodes key) {
        // keyDown overwrites keyBackClicked, how fun
        if (key == 27) keyBackClicked();
    }
    virtual void FLAlert_Clicked(gd::FLAlertLayer* alert, bool btn2);
};