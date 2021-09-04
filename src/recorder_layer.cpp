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
    bg->setContentSize({250, 200});
    bg->setPosition(win_size / 2);
    m_pLayer->addChild(bg);

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
    toggler->setPosition(center - ccp(98.0f, -70.0f));
    toggler->toggle(rs.recorder.m_recording);
    auto label = CCLabelBMFont::create("Record", "bigFont.fnt");
    label->setPosition(center - ccp(73.f, -70.f));
    label->setAnchorPoint({0, 0.5f});
    menu->addChild(toggler);
    layer->addChild(label);

    toggler = gd::CCMenuItemToggler::create(check_off_sprite, check_on_sprite, this, menu_selector(RecorderLayer::on_toggle_until_end));
    toggler->setPosition(center - ccp(98.0f, -35.0f));
    toggler->toggle(rs.recorder.m_until_end);
    label = CCLabelBMFont::create("Record until the end", "bigFont.fnt");
    label->limitLabelWidth(180.f, 1.f, 0.1f);
    label->setPosition(center - ccp(73.f, -35.f));
    label->setAnchorPoint({0, 0.5f});
    menu->addChild(toggler);
    layer->addChild(label);

    auto input = NumberInputNode::create({70.f, 30.f});
    input->set_value(rs.recorder.m_width);
    input->setPosition(center - ccp(79, 0));
    input->input_node->setMaxLabelScale(0.73f);
    input->callback = [](auto input) {
        ReplaySystem::get_instance().recorder.m_width = input->get_value();
    };
    layer->addChild(input);

    input = NumberInputNode::create({70.f, 30.f});
    input->set_value(rs.recorder.m_height);
    input->setPosition(center - ccp(4, 0));
    input->input_node->setMaxLabelScale(0.73f);
    input->callback = [&rs](auto input) {
        rs.recorder.m_height = input->get_value();
    };
    layer->addChild(input);

    input = NumberInputNode::create({50.f, 30.f});
    input->set_value(rs.recorder.m_fps);
    input->setPosition(center + ccp(76, 0));
    input->input_node->setMaxLabelScale(0.73f);
    input->callback = [&rs](auto input) {
        rs.recorder.m_fps = input->get_value();
    };
    layer->addChild(input);

    auto btn = gd::CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("gj_folderBtn_001.png"),
        this,
        menu_selector(RecorderLayer::on_pick_path)
    );
    btn->setPosition(center - ccp(98.f, 63.f));
    menu->addChild(btn);

    registerWithTouchDispatcher();
    CCDirector::sharedDirector()->getTouchDispatcher()->incrementForcePrio(2);

    auto close_btn = gd::CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png"),
        this,
        menu_selector(RecorderLayer::on_close)
    );

    m_pButtonMenu->addChild(close_btn);
    close_btn->setPosition(50.f, win_size.height - 50.f);

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
    if (static_cast<gd::CCMenuItemToggler*>(obj)->isOn()) {
        rs.recorder.stop();
    } else {
        // TODO: warn the user? idk theyre kinda dumb
        if (!rs.is_playing())
            rs.toggle_playing();
        rs.recorder.start();
    }
}

void RecorderLayer::on_toggle_until_end(CCObject* obj) {
    ReplaySystem::get_instance().recorder.m_until_end = !static_cast<gd::CCMenuItemToggler*>(obj)->isOn();
}

void RecorderLayer::on_pick_path(CCObject*) {
    nfdchar_t* path = nullptr;
    if (NFD_SaveDialog("mp4", nullptr, &path) == NFD_OKAY) {
        ReplaySystem::get_instance().recorder.m_output_path = std::string(path) + ".mp4";
        std::cout << "set path to " << ReplaySystem::get_instance().recorder.m_output_path  << std::endl;
        free(path);
    }
}