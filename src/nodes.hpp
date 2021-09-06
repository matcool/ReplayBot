#pragma once
#include "includes.h"
#include <gd.h>
#include <functional>
#include <stdexcept>

#define GEN_CREATE(class_name) template <typename... Args> \
static auto create(Args... args) { \
    auto node = new class_name; \
    if (node && node->init(args...)) \
        node->autorelease(); \
    else \
        CC_SAFE_DELETE(node); \
    return node; \
}

class TextInputNode : public CCNode, public gd::TextInputDelegate {
public:
    gd::CCTextInputNode* input_node;
    cocos2d::extension::CCScale9Sprite* background;
    std::function<void(TextInputNode*)> callback = [](auto){};

    GEN_CREATE(TextInputNode)

    bool init(CCSize size, float scale = 1.f, const std::string& font = "bigFont.fnt") {
        if (!CCNode::init()) return false;

        input_node = gd::CCTextInputNode::create("", this, font.c_str(), size.width, size.height);
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

    virtual void textChanged(gd::CCTextInputNode*) { callback(this); }

    void set_value(const std::string& value) { input_node->setString(value.c_str()); }
    std::string get_value() { return input_node->getString(); }
};

class NumberInputNode : public TextInputNode {
public:
    std::function<void(NumberInputNode*)> callback = [](auto){};
    
    GEN_CREATE(NumberInputNode)

    bool init(CCSize size, float scale = 1.f, const std::string& font = "bigFont.fnt") {
        if (!TextInputNode::init(size, scale, font)) return false;
        input_node->setAllowedChars("0123456789");
        return true;
    }
    virtual void textChanged(gd::CCTextInputNode*) { callback(this); }

    void set_value(int value) { input_node->setString(std::to_string(value).c_str()); }
    int get_value() {
        try {
            return std::stoi(input_node->getString());
        } catch (const std::invalid_argument&) {
            return -1;
        }
    }
};

template <typename T>
class NodeFactory {
public:
    static auto& start(T* node) { return *reinterpret_cast<NodeFactory*>(node); }
    
    template <typename... Args>
    static auto& start(Args... args) { return *reinterpret_cast<NodeFactory*>(T::create(args...)); }
    
    operator T*() { return reinterpret_cast<T*>(this); }

    #define _gen_func(name) template <typename... Args> \
    inline auto& name(Args... args) { \
        reinterpret_cast<T*>(this)->name(args...); \
        return *this; \
    }

    _gen_func(setPosition)
    _gen_func(setScale)
    _gen_func(setContentSize)
    _gen_func(setOpacity)
    _gen_func(setZOrder)
    _gen_func(setAnchorPoint)

    #undef _gen_func
};