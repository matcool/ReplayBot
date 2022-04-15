#include "nodes.hpp"

bool TextInputNode::init(CCSize size, float scale, const std::string& font) {
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

void TextInputNode::textChanged(gd::CCTextInputNode*) {
    callback(*this);
}

void TextInputNode::set_value(const std::string& value) {
    input_node->setString(value.c_str());
}

std::string TextInputNode::get_value() {
    return input_node->getString();
}


bool NumberInputNode::init(CCSize size, float scale, const std::string& font) {
    if (!TextInputNode::init(size, scale, font)) return false;
    input_node->setAllowedChars("0123456789");
    return true;
}

void NumberInputNode::textChanged(gd::CCTextInputNode*) {
    callback(*this);
}

void NumberInputNode::set_value(int value) {
    input_node->setString(std::to_string(value).c_str());
}

int NumberInputNode::get_value() {
    try {
        return std::stoi(input_node->getString());
    } catch (const std::invalid_argument&) {
        return -1;
    }
}


bool FloatInputNode::init(CCSize size, float scale, const std::string& font) {
    if (!TextInputNode::init(size, scale, font)) return false;
    input_node->setAllowedChars("0123456789.");
    return true;
}

void FloatInputNode::textChanged(gd::CCTextInputNode*) {
    callback(*this);
}

void FloatInputNode::set_value(float value) {
    // TODO: not hardcode this
    std::array<char, 10> arr;
    const auto result = std::to_chars(arr.data(), arr.data() + arr.size(), value, std::chars_format::fixed);
    if (result.ec == std::errc::value_too_large) {
        // TODO: have this be adjustable
        input_node->setString("1");
    } else {
        *result.ptr = 0;
        input_node->setString(arr.data());
    }
}

std::optional<float> FloatInputNode::get_value() {
    const auto str = input_node->getString();
    float value = 0.f;
    const auto result = std::from_chars(str, str + std::strlen(str), value);
    if (result.ec == std::errc::invalid_argument)
        return {};
    return value;
}