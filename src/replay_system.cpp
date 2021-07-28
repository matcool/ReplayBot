#include "replay_system.hpp"
#include "hooks.hpp"

void ReplaySystem::record_action(bool hold, bool player1, bool flip) {
    if (is_recording()) {
        auto gm = gd::GameManager::sharedState();
        auto play_layer = gm->getPlayLayer();
        auto is_two_player = play_layer->m_levelSettings->m_twoPlayerMode;
        player1 ^= flip && gm->getGameVariable("0010");
        Action action;
        action.hold = hold;
        action.player2 = is_two_player && !player1;
        if (replay.get_type() == ReplayType::XPOS)
            action.x = play_layer->m_player1->position.x;
        else if (replay.get_type() == ReplayType::FRAME)
            action.frame = get_frame();
        replay.add_action(action);
    }
}

void ReplaySystem::play_action(const Action& action) {
    auto gm = gd::GameManager::sharedState();
    auto flip = gm->getGameVariable("0010");
    auto func = action.hold ? Hooks::PlayLayer::pushButton : Hooks::PlayLayer::releaseButton;
    func(gm->getPlayLayer(), 0, !action.player2 ^ flip);
    ReplaySystem::play_sound(action.hold);
}

// Sounds must be in "Geometry Dash directory/clicks/"
// "down" - mouse down sounds 
// "up" - mouse up sounds 
void ReplaySystem::play_sound(bool hold) {
    std::string path = std::filesystem::current_path().string() + "/clicks/";
    if (hold)
        path += "down/";
    else
        path += "up/";

    srand(time(0));

    int files = dir_files_count(path);
    if (files == 0) {
        return;
    }

    int random = rand() % files;

    path += std::to_string(random) + ".ogg";

    if (std::filesystem::is_regular_file(path)) {
        gd::GameSoundManager::sharedState()->playSound(path);
    }
}

unsigned ReplaySystem::get_frame() {
    auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
    if (play_layer != nullptr) {
       return static_cast<unsigned>(play_layer->time * replay.get_fps()) + frame_offset;
    }
    return 0;
}

void ReplaySystem::on_reset() {
    auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
    if (is_playing()) {
        Hooks::PlayLayer::releaseButton(play_layer, 0, false);
        Hooks::PlayLayer::releaseButton(play_layer, 0, true);
        action_index = 0;
    } else if (is_recording()) {
        if (practice_fixes.checkpoints.empty()) {
            practice_fixes.activated_objects.clear();
            practice_fixes.activated_objects_p2.clear();
            frame_offset = 0;
        } else {
            frame_offset = practice_fixes.checkpoints.top().frame;
            constexpr auto delete_from = [&](auto& vec, size_t index) {
                vec.erase(vec.begin() + index, vec.end());
            };
            const auto& checkpoint = practice_fixes.checkpoints.top();
            delete_from(practice_fixes.activated_objects, checkpoint.activated_objects_size);
            delete_from(practice_fixes.activated_objects_p2, checkpoint.activated_objects_p2_size);
            for (const auto& object : practice_fixes.activated_objects) {
                object->m_hasBeenActivated = true;
            }
            for (const auto& object : practice_fixes.activated_objects_p2) {
                object->m_hasBeenActivatedP2 = true;
            }
        }
        if (replay.get_type() == ReplayType::XPOS)
            replay.remove_actions_after(play_layer->m_player1->position.x);
        else
            replay.remove_actions_after(get_frame());
        const auto& actions = replay.get_actions();
        bool holding = play_layer->m_player1->m_isHolding;
        if ((holding && actions.empty()) || (!actions.empty() && actions.back().hold != holding)) {
            record_action(holding, true, false);
            if (holding) {
                Hooks::PlayLayer::releaseButton(play_layer, 0, true);
                Hooks::PlayLayer::pushButton(play_layer, 0, true);
                play_layer->m_player1->m_hasJustHeld = true;
            }
        } else if (!actions.empty() && actions.back().hold && holding && !practice_fixes.checkpoints.empty() && practice_fixes.checkpoints.top().player1.buffer_orb) {
            Hooks::PlayLayer::releaseButton(play_layer, 0, true);
            Hooks::PlayLayer::pushButton(play_layer, 0, true);
        }
        if (play_layer->m_levelSettings->m_twoPlayerMode)
            record_action(false, false, false);
        practice_fixes.apply_checkpoint();
    }
}

void ReplaySystem::handle_playing() {
    if (is_playing()) {
        auto x = gd::GameManager::sharedState()->getPlayLayer()->m_player1->position.x;
        auto& actions = replay.get_actions();
        Action action;
        if (replay.get_type() == ReplayType::XPOS) {
            while (action_index < actions.size() && x >= (action = actions[action_index]).x) {
                play_action(action);
                ++action_index;
            }
        } else {
            while (action_index < actions.size() && get_frame() >= (action = actions[action_index]).frame) {
                play_action(action);
                ++action_index;
            }
        }
        if (action_index >= actions.size())
            toggle_playing();
    }
}

// why here out of all places? idk

constexpr int STATUS_LABEL_TAG = 10032;

auto _create_status_label(CCLayer* layer) {
    auto label = CCLabelBMFont::create("", "chatFont.fnt");
    label->setTag(STATUS_LABEL_TAG);
    label->setAnchorPoint({0, 0});
    label->setPosition({5, 5});
    label->setZOrder(999);
    layer->addChild(label);
    return label;
}

void ReplaySystem::_update_status_label() {
    auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
    if (play_layer) {
        auto label = cast<CCLabelBMFont*>(play_layer->getChildByTag(STATUS_LABEL_TAG));
        if (!label)
            label = _create_status_label(play_layer);
        switch (state) {
            case NOTHING:
                label->setString("");
                break;
            case RECORDING:
                label->setString("Recording");
                break;
            case PLAYING:
                label->setString("Playing");
                break;
        }
    }
}