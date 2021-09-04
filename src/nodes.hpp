#pragma once
#include "includes.h"
#include <gd.h>
#include <functional>
#include <stdexcept>

class NumberInputNode : public CCNode, public gd::TextInputDelegate {
public:
    gd::CCTextInputNode* input_node;
    cocos2d::extension::CCScale9Sprite* background;
    std::function<void(NumberInputNode*)> callback = [](auto){};

    static auto create(CCSize size, float bg_scale = 1.f) {
        auto node = new NumberInputNode;
        if (node && node->init(size, bg_scale)) {
            node->autorelease();
        } else {
            CC_SAFE_DELETE(node);
        }
        return node;
    }

    bool init(CCSize size, float scale) {
        if (!CCNode::init()) return false;

        input_node = gd::CCTextInputNode::create("", this, "bigFont.fnt", size.width, size.height);
        input_node->setAllowedChars("0123456789");
        input_node->setDelegate(this);
        addChild(input_node);

        background = extension::CCScale9Sprite::create("square02_small.png");
        background->setContentSize(size * scale);
        background->setScale(1.f / scale);
        background->setOpacity(100);
        background->setZOrder(-1);
        addChild(background);

        return true;
    }

    virtual void textChanged(gd::CCTextInputNode*) {
        callback(this);
    }

    void set_value(int value) {
        input_node->setString(std::to_string(value).c_str());
    }

    int get_value() {
        try {
            return std::stoi(input_node->getString());
        } catch (const std::invalid_argument&) {
            return -1;
        }
    }
};