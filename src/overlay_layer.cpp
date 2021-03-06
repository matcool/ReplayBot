#include "overlay_layer.hpp"
#include "replay_system.hpp"
#include <nfd.h>
#include <sstream>

bool OverlayLayer::init() {
    if (!initWithColor({ 0, 0, 0, 105 })) return false;

    setZOrder(20);

    auto win_size = CCDirector::sharedDirector()->getWinSize();
    auto& rs = ReplaySystem::get_instance();
        
    auto menu = CCMenu::create();
    menu->setPosition({0, win_size.height});
    addChild(menu);

    this->registerWithTouchDispatcher();
    CCDirector::sharedDirector()->getTouchDispatcher()->incrementForcePrio(2);

    auto sprite = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
    sprite->setScale(0.75f);
    
    auto btn = gd::CCMenuItemSpriteExtra::create(sprite, this, menu_selector(OverlayLayer::close_btn_callback));
    btn->setPosition({18, -18});
    menu->addChild(btn);

    sprite = CCSprite::create("GJ_button_01.png");
    sprite->setScale(0.72f);
    
    btn = gd::CCMenuItemSpriteExtra::create(sprite, this, menu_selector(OverlayLayer::on_record));
    btn->setPosition({35, -50});
    menu->addChild(btn);

    auto label = CCLabelBMFont::create("Record", "bigFont.fnt");
    label->setAnchorPoint({0, 0.5});
    label->setScale(0.8f);
    label->setPosition({55, win_size.height - 50});
    addChild(label);

    btn = gd::CCMenuItemSpriteExtra::create(sprite, this, menu_selector(OverlayLayer::on_play));
    btn->setPosition({35, -85});
    menu->addChild(btn);

    label = CCLabelBMFont::create("Play", "bigFont.fnt");
    label->setAnchorPoint({0, 0.5});
    label->setScale(0.8f);
    label->setPosition({55, win_size.height - 85});
    addChild(label);

    sprite = CCSprite::create("GJ_button_02.png");
    sprite->setScale(0.72f);

    btn = gd::CCMenuItemSpriteExtra::create(sprite, this, menu_selector(OverlayLayer::on_save));
    btn->setPosition({win_size.width - 35, -50});
    menu->addChild(btn);

    label = CCLabelBMFont::create("Save", "bigFont.fnt");
    label->setAnchorPoint({1, 0.5});
    label->setScale(0.8f);
    label->setPosition({win_size.width - 55, win_size.height - 50});
    addChild(label);

    btn = gd::CCMenuItemSpriteExtra::create(sprite, this, menu_selector(OverlayLayer::on_load));
    btn->setPosition({win_size.width - 35, -85});
    menu->addChild(btn);

    label = CCLabelBMFont::create("Load", "bigFont.fnt");
    label->setAnchorPoint({1, 0.5});
    label->setScale(0.8f);
    label->setPosition({win_size.width - 55, win_size.height - 85});
    addChild(label);

    auto check_off_sprite = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    auto check_on_sprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");

    m_x_pos_toggle = gd::CCMenuItemToggler::create(check_off_sprite, check_on_sprite, this, menu_selector(OverlayLayer::on_x_pos));
    m_x_pos_toggle->setPosition({win_size.width - 35, -120});
    menu->addChild(m_x_pos_toggle);

    m_frame_toggle = gd::CCMenuItemToggler::create(check_off_sprite, check_on_sprite, this, menu_selector(OverlayLayer::on_frame));
    m_frame_toggle->setPosition({win_size.width - 35, -155});
    menu->addChild(m_frame_toggle);

    if (rs.get_default_type() == ReplayType::XPOS)
        m_x_pos_toggle->toggle(true);
    else
        m_frame_toggle->toggle(true);

    label = CCLabelBMFont::create("X Pos", "bigFont.fnt");
    label->setAnchorPoint({1, 0.5});
    label->setScale(0.8f);
    label->setPosition({win_size.width - 55, win_size.height - 120});
    addChild(label);

    label = CCLabelBMFont::create("Frame", "bigFont.fnt");
    label->setAnchorPoint({1, 0.5});
    label->setScale(0.8f);
    label->setPosition({win_size.width - 55, win_size.height - 155});
    addChild(label);

    auto toggle = gd::CCMenuItemToggler::create(check_off_sprite, check_on_sprite, this, menu_selector(OverlayLayer::on_toggle_real_time));
    toggle->setPosition({win_size.width - 35, -190});
    toggle->toggle(rs.real_time_mode);
    menu->addChild(toggle);

    label = CCLabelBMFont::create("Real Time", "bigFont.fnt");
    label->setAnchorPoint({1, 0.5});
    label->setScale(0.8f);
    label->setPosition({win_size.width - 55, win_size.height - 190});
    addChild(label);

    sprite = CCSprite::create("square02b_001.png");
    sprite->setColor({0, 0, 0});
    sprite->setOpacity(69);
    sprite->setPosition({110, win_size.height - 115});
    sprite->setScaleX(0.825f);
    sprite->setScaleY(0.325f);
    sprite->setZOrder(-1);
    addChild(sprite);

    m_fps_input = gd::CCTextInputNode::create("fps", nullptr, "bigFont.fnt", 100.f, 100.f);
    m_fps_input->setString(std::to_string(static_cast<int>(rs.get_default_fps())).c_str());
    m_fps_input->setLabelPlaceholderColor({200, 200, 200});
    m_fps_input->setLabelPlaceholerScale(0.5f);
    m_fps_input->setMaxLabelScale(0.7f);
    m_fps_input->setMaxLabelLength(0);
    m_fps_input->setAllowedChars("0123456789");
    m_fps_input->setMaxLabelLength(10);
    m_fps_input->setPosition({110, win_size.height - 115});
    addChild(m_fps_input);

    label = CCLabelBMFont::create("FPS:", "bigFont.fnt");
    label->setAnchorPoint({0, 0.5f});
    label->setScale(0.7f);
    label->setPosition({20, win_size.height - 115});
    addChild(label);

    m_replay_info = CCLabelBMFont::create("", "chatFont.fnt");
    m_replay_info->setAnchorPoint({0, 1});
    m_replay_info->setPosition({20, win_size.height - 133});
    update_info_text();
    addChild(m_replay_info);

    setKeypadEnabled(true);
    setTouchEnabled(true);

    return true;
}

void OverlayLayer::update_info_text() {
    auto& rs = ReplaySystem::get_instance();
    auto& replay = rs.get_replay();
    std::stringstream stream;
    stream << "Current Replay:\nFPS: " << replay.get_fps();
    stream << "\nActions: " << replay.get_actions().size();
    stream << "\nMode: " << (replay.get_type() == ReplayType::XPOS ? "X Pos" : "Frame");
    m_replay_info->setString(stream.str().c_str());
}

void OverlayLayer::_update_default_fps() {
    auto text = m_fps_input->getString();
    if (text[0])
        ReplaySystem::get_instance().set_default_fps(std::stof(text));
}

void OverlayLayer::FLAlert_Clicked(gd::FLAlertLayer* alert, bool btn2) {
    if (!btn2) {
        auto& rs = ReplaySystem::get_instance();
        int tag = alert->getTag();
        if (tag == 1) {
            rs.toggle_recording();
            update_info_text();
        } else if (tag == 2) {
            _handle_load_replay();
        }
        update_info_text();
    }
}

void OverlayLayer::on_record(CCObject*) {
    _update_default_fps();
    auto& rs = ReplaySystem::get_instance();
    if (!rs.is_recording()) {
        if (rs.get_replay().get_actions().empty()) {
            rs.toggle_recording();
            update_info_text();
        } else {
            auto alert = gd::FLAlertLayer::create(this,
            "Warning",
            "Ok",
            "Cancel",
            "This will <cr>overwrite</c> your currently loaded replay.");
            alert->setTag(1);
            alert->show();
        }
    } else {
        rs.toggle_recording();
    }
}

void OverlayLayer::on_play(CCObject*) {
    ReplaySystem::get_instance().toggle_playing();
}

void OverlayLayer::on_save(CCObject*) {
    nfdchar_t* path = nullptr;
    auto result = NFD_SaveDialog("replay", nullptr, &path);
    if (result == NFD_OKAY) {
        std::string s_path(path);
        // why doesnt it just add the extension for me
        ReplaySystem::get_instance().get_replay().save(s_path + ".replay");
        gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Replay saved.")->show();
        free(path);
    }
}

void OverlayLayer::_handle_load_replay() {
    nfdchar_t* path = nullptr;
    auto result = NFD_OpenDialog("replay", nullptr, &path);
    if (result == NFD_OKAY) {
        ReplaySystem::get_instance().get_replay() = Replay::load(path);
        update_info_text();
        gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Replay loaded.")->show();
        free(path);
    }
}

void OverlayLayer::on_load(CCObject*) {
    auto& rs = ReplaySystem::get_instance();
    if (rs.get_replay().get_actions().empty()) {
        _handle_load_replay();
    } else {
        auto alert = gd::FLAlertLayer::create(this,
        "Warning",
        "Ok",
        "Cancel",
        "This will <cr>overwrite</c> your currently loaded replay.");
        alert->setTag(2);
        alert->show();
    }
}

void OverlayLayer::keyBackClicked() {
    _update_default_fps();
    gd::FLAlertLayer::keyBackClicked();
}

void OverlayLayer::on_x_pos(CCObject*) {
    m_x_pos_toggle->toggle(false);
    m_frame_toggle->toggle(false);
    ReplaySystem::get_instance().set_default_type(ReplayType::XPOS);
}

void OverlayLayer::on_frame(CCObject*) {
    m_x_pos_toggle->toggle(false);
    m_frame_toggle->toggle(false);
    ReplaySystem::get_instance().set_default_type(ReplayType::FRAME);
}

void OverlayLayer::on_toggle_real_time(CCObject* toggle_) {
    auto toggle = cast<gd::CCMenuItemToggler*>(toggle_);
    if (toggle != nullptr) {
        ReplaySystem::get_instance().real_time_mode = !toggle->isOn(); // why is it flipped
    }
}