#include "recorder_layer.hpp"
#include "nodes.hpp"
#include "replay_system.hpp"
#include <nfd.h>

bool RecorderLayer::init() {
    auto win_size = cocos2d::CCDirector::sharedDirector()->getWinSize();

    if (!initWithColor({0, 0, 0, 105})) return false;
    m_pLayer = CCLayer::create();
    addChild(m_pLayer);

    auto bg = cocos2d::extension::CCScale9Sprite::create("GJ_square01.png", { 0.0f, 0.0f, 80.0f, 80.0f });
    const CCSize window_size(400, 250);
    bg->setContentSize(window_size);
    bg->setPosition(win_size / 2);
    m_pLayer->addChild(bg);

    const CCPoint top_left = win_size / 2.f - ccp(window_size.width / 2.f, -window_size.height / 2.f);

    m_pButtonMenu = CCMenu::create();
    m_pButtonMenu->setPosition({0, 0});
    auto menu = m_pButtonMenu; // sorry m_pButtonMenu is too much to type
    m_pLayer->addChild(m_pButtonMenu);
    auto layer = m_pLayer;

    auto check_off_sprite = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    auto check_on_sprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");

    const auto center = win_size / 2.f;

    auto& rs = ReplaySystem::get_instance();

    auto toggler = gd::CCMenuItemToggler::create(check_off_sprite, check_on_sprite, this, menu_selector(RecorderLayer::on_toggle_recorder));
    toggler->setPosition(top_left + ccp(30.f, -30.f));
    toggler->toggle(rs.recorder.m_recording);
    auto label = CCLabelBMFont::create("Record", "bigFont.fnt");
    label->setPosition(top_left + ccp(55.f, -30.f));
    label->setScale(0.7f);
    label->setAnchorPoint({0, 0.5f});
    menu->addChild(toggler);
    layer->addChild(label);

    toggler = gd::CCMenuItemToggler::create(check_off_sprite, check_on_sprite, this, menu_selector(RecorderLayer::on_toggle_until_end));
    toggler->setPosition(top_left + ccp(30.f, -65.f));
    toggler->toggle(rs.recorder.m_until_end);
    label = CCLabelBMFont::create("Record until the end", "bigFont.fnt");
    label->setScale(0.7f);
    label->setPosition(top_left + ccp(55.f, -65.f));
    label->setAnchorPoint({0, 0.5f});
    menu->addChild(toggler);
    layer->addChild(label);

    auto input = NumberInputNode::create(CCSize(70.f, 30.f));
    input->set_value(rs.recorder.m_width);
    input->setPosition(top_left + ccp(49.f, -104.f));
    input->input_node->setMaxLabelScale(0.73f);
    input->callback = [&rs](auto input) {
        rs.recorder.m_width = input->get_value();
    };
    layer->addChild(input);

    layer->addChild(NodeFactory<CCLabelBMFont>::start("x", "bigFont.fnt")
                    .setPosition(top_left + ccp(93.5f, -104.f))
                    .setScale(0.5f));

    input = NumberInputNode::create(CCSize(70.f, 30.f));
    input->set_value(rs.recorder.m_height);
    input->setPosition(top_left + ccp(137.f, -104.f));
    input->input_node->setMaxLabelScale(0.73f);
    input->callback = [&rs](auto input) {
        rs.recorder.m_height = input->get_value();
    };
    layer->addChild(input);

    layer->addChild(NodeFactory<CCLabelBMFont>::start("@", "bigFont.fnt")
                    .setPosition(top_left + ccp(185.5f, -104.f))
                    .setScale(0.5f));

    input = NumberInputNode::create(CCSize(50.f, 30.f));
    input->set_value(rs.recorder.m_fps);
    input->setPosition(top_left + ccp(225.f, -104.f));
    input->input_node->setMaxLabelScale(0.73f);
    input->callback = [&rs](auto input) {
        rs.recorder.m_fps = input->get_value();
    };
    layer->addChild(input);

    const std::string broad_filter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.,;-_=+@!\":0123456789$[](){}";

    auto text_input = TextInputNode::create(CCSize(60.f, 30.f), 1.f, "chatFont.fnt");
    text_input->setPosition(top_left + ccp(291.5f, -177.f));
    text_input->input_node->setAllowedChars(broad_filter);
    text_input->set_value(rs.recorder.m_bitrate);
    text_input->callback = [&rs](auto input) {
        rs.recorder.m_bitrate = input->get_value();
    };
    layer->addChild(text_input);

    text_input = TextInputNode::create(CCSize(60.f, 30.f), 1.f, "chatFont.fnt");
    text_input->setPosition(top_left + ccp(359.5f, -177.f));
    text_input->input_node->m_sCaption = "Default";
    text_input->input_node->setAllowedChars(broad_filter);
    text_input->input_node->setLabelPlaceholderColor({200, 200, 200});
    text_input->set_value(rs.recorder.m_codec);
    text_input->callback = [&rs](auto input) {
        rs.recorder.m_codec = input->get_value();
    };
    layer->addChild(text_input);

    text_input = TextInputNode::create(CCSize(128.f, 30.f), 1.f, "chatFont.fnt");
    text_input->setPosition(top_left + ccp(324.5f, -217.f));
    text_input->input_node->m_sCaption = "Extra options";
    text_input->input_node->setAllowedChars(broad_filter);
    text_input->set_value(rs.recorder.m_extra_args);
    text_input->callback = [&rs](auto input) {
        rs.recorder.m_extra_args = input->get_value();
    };
    text_input->input_node->setLabelPlaceholderColor({200, 200, 200});
    layer->addChild(text_input);

    layer->addChild(NodeFactory<CCLabelBMFont>::start("Bitrate", "bigFont.fnt").setPosition(top_left + ccp(291.5f, -152.f)).setScale(0.4f));
    layer->addChild(NodeFactory<CCLabelBMFont>::start("Codec", "bigFont.fnt").setPosition(top_left + ccp(359.5f, -152.f)).setScale(0.4f));

    registerWithTouchDispatcher();
    CCDirector::sharedDirector()->getTouchDispatcher()->incrementForcePrio(2);

    auto close_btn = gd::CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this,
        menu_selector(RecorderLayer::on_close)
    );

    m_pButtonMenu->addChild(close_btn);
    close_btn->setPosition(18.f, win_size.height - 18.f);
    close_btn->getNormalImage()->setScale(.75f);

    setKeypadEnabled(true);
    setTouchEnabled(true);

    return true;
}

void RecorderLayer::keyBackClicked() {
    setKeyboardEnabled(false);
    removeFromParentAndCleanup(true);
}

void RecorderLayer::on_close(CCObject*) {
    keyBackClicked();
}

void RecorderLayer::on_toggle_recorder(CCObject* obj) {
    auto& rs = ReplaySystem::get_instance();
    auto toggler = static_cast<gd::CCMenuItemToggler*>(obj);
    if (toggler->isOn()) {
        rs.recorder.stop();
    } else {
        nfdchar_t* path = nullptr;
        if (NFD_SaveDialog("mp4", nullptr, &path) == NFD_OKAY) {
            // TODO: warn the user? idk theyre kinda dumb
            if (!rs.is_playing())
                rs.toggle_playing();
            std::string p = std::string(path) + ".mp4";
            std::cout << "saving it to " << p << std::endl;
            rs.recorder.start(p);
            free(path);
        } else {
            // toggle it on so then gd does !on and then turns it off then boom success
            toggler->toggle(true);
        }
    }
}

void RecorderLayer::on_toggle_until_end(CCObject* obj) {
    ReplaySystem::get_instance().recorder.m_until_end = !static_cast<gd::CCMenuItemToggler*>(obj)->isOn();
}