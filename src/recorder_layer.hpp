#pragma once
#include "includes.h"

class RecorderLayer : public gd::FLAlertLayer {
public:
    CCLabelBMFont* m_path_label;

    static auto create() {
        auto node = new RecorderLayer;
        if (node && node->init()) {
            node->autorelease();
        } else {
            CC_SAFE_DELETE(node);
        }
        return node;
    }
    bool init();
    void keyBackClicked();
    void on_close(CCObject*);
    void on_toggle_recorder(CCObject*);
    void on_toggle_until_end(CCObject*);
    void on_toggle_include_audio(CCObject*);
};