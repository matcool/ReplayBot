#include "overlay_layer.hpp"
#include "replay_system.hpp"
#include <nfd.h>
#include <sstream>
#include "recorder_layer.hpp"
#include "nodes.hpp"
#include "version.hpp"
#include <filesystem>

bool OverlayLayer::init() {
    if (!initWithColor({ 0, 0, 0, 105 })) return false;

    setZOrder(20);

    auto win_size = CCDirector::sharedDirector()->getWinSize();
    auto& rs = ReplaySystem::get();
        
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

    // TODO: make these toggles    
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

    auto toggle = gd::CCMenuItemToggler::create(check_off_sprite, check_on_sprite, this, menu_selector(OverlayLayer::on_toggle_real_time));
    toggle->setPosition({win_size.width - 35, -120});
    toggle->toggle(rs.real_time_mode);
    menu->addChild(toggle);

    label = CCLabelBMFont::create("Real Time", "bigFont.fnt");
    label->setAnchorPoint({1, 0.5});
    label->setScale(0.6f);
    label->setPosition({win_size.width - 55, win_size.height - 120});
    addChild(label);

    // TODO: add some info button to nodes.hpp
    menu->addChild(
        NodeFactory<gd::CCMenuItemSpriteExtra>::start(
            NodeFactory<CCSprite>::start(CCSprite::createWithSpriteFrameName("GJ_infoIcon_001.png")).setScale(.525f).finish(),
            this, menu_selector(OverlayLayer::on_info_real_time)
        )
        .setPosition(ccp(win_size.width - 11, -109))
        .setZOrder(-1)
    );

    toggle = gd::CCMenuItemToggler::create(check_off_sprite, check_on_sprite, this, menu_selector(OverlayLayer::on_toggle_showcase));
    toggle->setPosition({win_size.width - 35, -155});
    toggle->toggle(!rs.showcase_mode);
    menu->addChild(toggle);

    label = CCLabelBMFont::create("Status Text", "bigFont.fnt");
    label->setAnchorPoint({1, 0.5});
    label->setScale(0.6f);
    label->setPosition({win_size.width - 55, win_size.height - 155});
    addChild(label);

    btn = gd::CCMenuItemSpriteExtra::create(CCSprite::create("GJ_button_01.png"), this, menu_selector(OverlayLayer::on_recorder));
    {
        auto draw_node = CCDrawNode::create();
        constexpr size_t n_verts = 16;
        constexpr float radius = 13.f;
        CCPoint verts[n_verts];
        for (size_t i = 0; i < n_verts; ++i) {
            verts[i] = CCPoint::forAngle(static_cast<float>(i) / n_verts * 6.2831f) * radius;
        }
        draw_node->drawPolygon(verts, n_verts, {1.f, 0.f, 0.f, 1.f}, 1.f, {0.f, 0.f, 0.f, 1.f});
        btn->getNormalImage()->addChild(draw_node);
        draw_node->setPosition(btn->getNormalImage()->getContentSize() / 2.f);
    }
    btn->getNormalImage()->setScale(0.775f);
    btn->setPosition({win_size.width - 35, -190});
    menu->addChild(btn);

    addChild(NodeFactory<CCLabelBMFont>::start("Internal Renderer", "bigFont.fnt")
        .setAnchorPoint(ccp(1, 0.5))
        .setScale(0.6f)
        .setPosition(win_size - ccp(55, 190))
    );

    m_replay_info = CCLabelBMFont::create("", "chatFont.fnt");
    m_replay_info->setAnchorPoint({0, 1});
    m_replay_info->setPosition({20, win_size.height - 103});
    update_info_text();
    addChild(m_replay_info);
    
    addChild(NodeFactory<CCLabelBMFont>::start(REPLAYBOT_VERSION, "chatFont.fnt")
        .setAnchorPoint(ccp(1, 0))
        .setScale(0.6f)
        .setPosition(ccp(win_size.width - 5, 5))
        .setOpacity(100)
    );

    setKeypadEnabled(true);
    setTouchEnabled(true);

    return true;
}

void OverlayLayer::update_info_text() {
    auto& rs = ReplaySystem::get();
    auto& replay = rs.get_replay();
    std::stringstream stream;
    stream << "Current Replay:\nFPS: " << replay.get_fps();
    stream << "\nActions: " << replay.get_actions().size();
    stream << "\nMode: " << (replay.get_type() == ReplayType::XPOS ? "X Pos" : "Frame");
    m_replay_info->setString(stream.str().c_str());
}

void OverlayLayer::FLAlert_Clicked(gd::FLAlertLayer* alert, bool btn2) {
    if (!btn2) {
        if (alert->getTag() == 44) {
            CCApplication::sharedApplication()->openURL("https://www.gyan.dev/ffmpeg/builds/");
        } else {
            _handle_load_replay();
            update_info_text();
        }
    }
}

void OverlayLayer::on_record(CCObject*) {
    auto& rs = ReplaySystem::get();
    if (rs.is_recording())
        rs.toggle_recording();
    else
        RecordOptionsLayer::create(this)->show();
}

void OverlayLayer::on_play(CCObject*) {
    ReplaySystem::get().toggle_playing();
}

void OverlayLayer::on_save(CCObject*) {
    nfdchar_t* path = nullptr;
    auto result = NFD_SaveDialog("replay", nullptr, &path);
    if (result == NFD_OKAY) {
        ReplaySystem::get().get_replay().save(path);
        gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Replay saved.")->show();
        free(path);
    }
}

void OverlayLayer::_handle_load_replay() {
    nfdchar_t* path = nullptr;
    auto result = NFD_OpenDialog("replay", nullptr, &path);
    if (result == NFD_OKAY) {
        ReplaySystem::get().get_replay() = Replay::load(path);
        update_info_text();
        gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, "Replay loaded.")->show();
        free(path);
    }
}

void OverlayLayer::on_load(CCObject*) {
    auto& rs = ReplaySystem::get();
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
    gd::FLAlertLayer::keyBackClicked();
}

void OverlayLayer::on_toggle_real_time(CCObject* toggle_) {
    auto toggle = cast<gd::CCMenuItemToggler*>(toggle_);
    if (toggle != nullptr) {
        ReplaySystem::get().real_time_mode = !toggle->isOn(); // why is it flipped
    }
}

void OverlayLayer::on_toggle_showcase(CCObject* toggle_) {
    auto toggle = cast<gd::CCMenuItemToggler*>(toggle_);
    if (toggle != nullptr) {
        ReplaySystem::get().showcase_mode = toggle->isOn();
    }
}

void OverlayLayer::on_recorder(CCObject*) {
    static bool has_ffmpeg = false;
    if (!has_ffmpeg) {
        wchar_t buffer[MAX_PATH];
        GetModuleFileNameW(GetModuleHandleA(NULL), buffer, MAX_PATH);
        const auto path = std::filesystem::path(buffer).parent_path() / "ffmpeg.exe";
        if (std::filesystem::exists(path)) {
            has_ffmpeg = true;
            ReplaySystem::get().recorder.m_ffmpeg_path = path.string();
        } else {
            // theres prob a way to do it by not spawning a process but im lazy and hate dealing with winapi
            auto process = subprocess::Popen("where ffmpeg");
            if (process.close()) {
                auto popup = gd::FLAlertLayer::create(this, "Error",
                    "Download", "Cancel",
                    "ffmpeg was not found, recorder will not work without it. "
                    "To install ffmpeg download it and place the ffmpeg.exe (found inside the bin folder in the zip) in the gd folder"
                );
                popup->setTag(44);
                popup->show();
            } else
                has_ffmpeg = true;
            if (!has_ffmpeg)
                return;
        }
    }
    RecorderLayer::create()->show();
}

void OverlayLayer::on_info_real_time(CCObject*) {
    FLAlertLayer::create(nullptr, "Info", "OK", nullptr,
        "Will try to run the game at full speed even if the fps doesn't match with the replay's fps.\nOnly in effect when recording or playing.")->show();
}

bool RecordOptionsLayer::init(OverlayLayer* parent) {
    m_parent = parent;

    auto win_size = cocos2d::CCDirector::sharedDirector()->getWinSize();

    if (!initWithColor({0, 0, 0, 105})) return false;
    m_pLayer = CCLayer::create();
    addChild(m_pLayer);

    auto bg = cocos2d::extension::CCScale9Sprite::create("GJ_square01.png");
    const CCSize window_size(250, 150);
    bg->setContentSize(window_size);
    bg->setPosition(win_size / 2);
    bg->setZOrder(-5);
    m_pLayer->addChild(bg);

    const CCPoint top_left = win_size / 2.f - ccp(window_size.width / 2.f, -window_size.height / 2.f);
    std::cout << "top_left is " << top_left << std::endl;

    registerWithTouchDispatcher();
    CCDirector::sharedDirector()->getTouchDispatcher()->incrementForcePrio(2);

    m_pButtonMenu = CCMenu::create();
    m_pButtonMenu->setPosition(top_left);
    auto* const menu = m_pButtonMenu;
    m_pLayer->addChild(m_pButtonMenu);
    auto* const layer = m_pLayer;

    layer->addChild(
        NodeFactory<CCLabelBMFont>::start("Record mode", "bigFont.fnt")
        .setScale(0.5f)
        .setPosition(top_left + ccp(10.f, -17.f))
        .setAnchorPoint(ccp(0, 0.5f))
    );

    auto* const check_off_sprite = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
    check_off_sprite->setScale(0.6f);
    auto* const check_on_sprite = CCSprite::createWithSpriteFrameName("GJ_checkOn_001.png");
    check_on_sprite->setScale(0.6f);

    m_x_pos_toggle = gd::CCMenuItemToggler::create(check_off_sprite, check_on_sprite, this, menu_selector(RecordOptionsLayer::on_x_pos));
    m_x_pos_toggle->setPosition(20.f, -43.f);
    menu->addChild(m_x_pos_toggle);

    layer->addChild(
        NodeFactory<CCLabelBMFont>::start("X Pos", "bigFont.fnt")
        .setScale(0.5f)
        .setPosition(top_left + ccp(33, -42))
        .setAnchorPoint(ccp(0, .5f))
    );

    m_frame_toggle = gd::CCMenuItemToggler::create(check_off_sprite, check_on_sprite, this, menu_selector(RecordOptionsLayer::on_frame));
    m_frame_toggle->setPosition(150.f, -43.f);
    menu->addChild(m_frame_toggle);

    layer->addChild(
        NodeFactory<CCLabelBMFont>::start("Frame", "bigFont.fnt")
        .setScale(0.5f)
        .setPosition(top_left + ccp(163, -42))
        .setAnchorPoint(ccp(0, .5f))
    );

    auto& rs = ReplaySystem::get();

    if (rs.get_default_type() == ReplayType::XPOS)
        m_x_pos_toggle->toggle(true);
    else
        m_frame_toggle->toggle(true);

    layer->addChild(NodeFactory<CCLabelBMFont>::start("FPS", "bigFont.fnt")
        .setAnchorPoint(ccp(0, 0.5f))
        .setScale(0.7f)
        .setPosition(top_left + ccp(10, -75))
    );

    m_fps_input = NumberInputNode::create(CCSize(64.f, 30.f));

    m_fps_input->set_value(static_cast<int>(rs.get_default_fps()));
    m_fps_input->input_node->setMaxLabelScale(0.7f);
    m_fps_input->input_node->setMaxLabelLength(10);
    m_fps_input->setPosition(top_left + ccp(100, -75));
    layer->addChild(m_fps_input);

    menu->addChild(
        NodeFactory<gd::CCMenuItemSpriteExtra>::start(
            gd::ButtonSprite::create("Record", 90, true, "goldFont.fnt", "GJ_button_01.png", 30.f, 0.7f),
            this, menu_selector(RecordOptionsLayer::on_record)
        )
        .setPosition(ccp(125, -125))
    );

    menu->addChild(
        NodeFactory<gd::CCMenuItemSpriteExtra>::start(
            NodeFactory<CCSprite>::start(CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png")).setScale(.75f).finish(),
            this, menu_selector(RecordOptionsLayer::on_close)
        )
        .setPosition(ccp(18.f, win_size.height - 18.f) - top_left)
    );

    setKeypadEnabled(true);
    setTouchEnabled(true);
    setKeyboardEnabled(true);

    return true;
}

void RecordOptionsLayer::keyBackClicked() {
    update_default_fps();
    setKeyboardEnabled(false);
    removeFromParentAndCleanup(true);
}

void RecordOptionsLayer::on_close(CCObject*) {
    keyBackClicked();
}

void RecordOptionsLayer::on_x_pos(CCObject*) {
    m_x_pos_toggle->toggle(false);
    m_frame_toggle->toggle(false);
    ReplaySystem::get().set_default_type(ReplayType::XPOS);
}

void RecordOptionsLayer::on_frame(CCObject*) {
    m_x_pos_toggle->toggle(false);
    m_frame_toggle->toggle(false);
    ReplaySystem::get().set_default_type(ReplayType::FRAME);
}

void RecordOptionsLayer::update_default_fps() {
    ReplaySystem::get().set_default_fps(static_cast<float>(m_fps_input->get_value()));
}

void RecordOptionsLayer::on_record(CCObject*) {
    update_default_fps();
    auto& rs = ReplaySystem::get();
    if (!rs.is_recording()) {
        if (rs.get_replay().get_actions().empty()) {
            rs.toggle_recording();
            m_parent->update_info_text();
            this->keyBackClicked();
        } else {
            auto alert = gd::FLAlertLayer::create(
                this,
                "Warning",
                "Ok",
                "Cancel",
                "This will <cr>overwrite</c> your currently loaded replay."
            );
            alert->show();
        }
    } else {
        rs.toggle_recording();
    }
}

void RecordOptionsLayer::FLAlert_Clicked(gd::FLAlertLayer*, bool btn2) {
    if (!btn2) {
        auto& rs = ReplaySystem::get();
        rs.toggle_recording();
        m_parent->update_info_text();
        this->keyBackClicked();
    }
}