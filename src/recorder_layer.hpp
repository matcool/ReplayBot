#pragma once
#include "includes.h"
#include <gd.h>

class RecorderLayer : public gd::FLAlertLayer {
public:
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
    void on_pick_path(CCObject*);
};