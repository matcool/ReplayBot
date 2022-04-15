#pragma once
#include "includes.h"
#include <gd.h>
#include <functional>
#include <stdexcept>
#include <charconv>
#include <array>
#include <optional>

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
    std::function<void(TextInputNode&)> callback = [](auto&){};

    GEN_CREATE(TextInputNode)

    bool init(CCSize size, float scale = 1.f, const std::string& font = "bigFont.fnt");

    virtual void textChanged(gd::CCTextInputNode*);

    void set_value(const std::string& value);
    std::string get_value();
};

class NumberInputNode : public TextInputNode {
public:
    std::function<void(NumberInputNode&)> callback = [](auto&){};

    GEN_CREATE(NumberInputNode)

    bool init(CCSize size, float scale = 1.f, const std::string& font = "bigFont.fnt");
    virtual void textChanged(gd::CCTextInputNode*);

    void set_value(int value);
    int get_value();
};

class FloatInputNode : public TextInputNode {
public:
    std::function<void(FloatInputNode&)> callback = [](auto&){};
    
    GEN_CREATE(FloatInputNode)

    bool init(CCSize size, float scale = 1.f, const std::string& font = "bigFont.fnt");
    virtual void textChanged(gd::CCTextInputNode*);

    void set_value(float value);
    std::optional<float> get_value();
};

template <typename T>
class NodeFactory {
public:
    static auto& start(T* node) { return *reinterpret_cast<NodeFactory*>(node); }
    
    template <typename... Args>
    static auto& start(Args... args) { return *reinterpret_cast<NodeFactory*>(T::create(args...)); }
    
    T* finish() { return reinterpret_cast<T*>(this); }
    operator T*() { return finish(); }

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
    _gen_func(setColor)

    #undef _gen_func
};