#include "replay_system.hpp"
#include "hooks.hpp"
#include <filesystem>
#include <fstream>

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
            action.x = play_layer->m_player1->m_position.x;
        else if (replay.get_type() == ReplayType::FRAME)
            action.frame = get_frame();
        replay.add_action(action);
    }
}

void ReplaySystem::play_action(const Action& action) {
    auto gm = gd::GameManager::sharedState();
    auto flip = gm->getGameVariable("0010");
    if (action.hold) orig<&Hooks::PlayLayer_pushButton>(gm->getPlayLayer(), 0, !action.player2 ^ flip);
    else orig<&Hooks::PlayLayer_releaseButton>(gm->getPlayLayer(), 0, !action.player2 ^ flip);
}

unsigned ReplaySystem::get_frame() {
    auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
    if (play_layer != nullptr) {
       return static_cast<unsigned>(play_layer->m_time * replay.get_fps()) + frame_offset;
    }
    return 0;
}

void ReplaySystem::update_frame_offset() {
    // if there is no last checkpoint then it should default to 0
    frame_offset = practice_fixes.get_last_checkpoint().frame;
}

void ReplaySystem::toggle_playing() {
    state = is_playing() ? NOTHING : PLAYING;
    auto pl = gd::GameManager::sharedState()->getPlayLayer();
    if (is_playing())  {
        if (pl && get_frame() > 0)
            should_restart_next_time = true;
        action_index = 0;
    } else
        should_restart_next_time = false;
    update_frame_offset();
    _update_status_label();
}

void ReplaySystem::toggle_recording() {
    state = is_recording() ? NOTHING : RECORDING;
    if (!is_recording()) {
        frame_advance = false;
        should_restart_next_time = false;
    } else {
        replay = Replay(default_fps, default_type);
        if (gd::GameManager::sharedState()->getPlayLayer() && get_frame() > 0)
            should_restart_next_time = true;
    }
    update_frame_offset();
    _update_status_label();
}

void ReplaySystem::on_reset() {
    auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
    if (is_playing()) {
        update_frame_offset();
        orig<&Hooks::PlayLayer_releaseButton>(play_layer, 0, false);
        orig<&Hooks::PlayLayer_releaseButton>(play_layer, 0, true);
        action_index = 0;
        practice_fixes.activated_objects.clear();
        practice_fixes.activated_objects_p2.clear();
    } else {
        bool has_checkpoints = play_layer->m_checkpoints->count();
        const auto checkpoint = practice_fixes.get_last_checkpoint();
        if (!has_checkpoints) {
            practice_fixes.activated_objects.clear();
            practice_fixes.activated_objects_p2.clear();
            frame_offset = 0;
        } else {
            frame_offset = checkpoint.frame;
            constexpr auto delete_from = [&](auto& vec, size_t index) {
                vec.erase(vec.begin() + index, vec.end());
            };
            delete_from(practice_fixes.activated_objects, checkpoint.activated_objects_size);
            delete_from(practice_fixes.activated_objects_p2, checkpoint.activated_objects_p2_size);
            if (is_recording()) {
                for (const auto& object : practice_fixes.activated_objects) {
                    object->m_hasBeenActivated = true;
                }
                for (const auto& object : practice_fixes.activated_objects_p2) {
                    object->m_hasBeenActivatedP2 = true;
                }
            }
        }
        if (is_recording()) {
            if (replay.get_type() == ReplayType::XPOS)
                replay.remove_actions_after(play_layer->m_player1->m_position.x);
            else
                replay.remove_actions_after(get_frame());
            const auto& actions = replay.get_actions();
            bool holding = play_layer->m_player1->m_isHolding;
            if ((holding && actions.empty()) || (!actions.empty() && actions.back().hold != holding)) {
                record_action(holding, true, false);
                if (holding) {
                    orig<&Hooks::PlayLayer_releaseButton>(play_layer, 0, true);
                    orig<&Hooks::PlayLayer_pushButton>(play_layer, 0, true);
                    play_layer->m_player1->m_hasJustHeld = true;
                }
            } else if (!actions.empty() && actions.back().hold && holding && has_checkpoints && checkpoint.player1.buffer_orb) {
                orig<&Hooks::PlayLayer_releaseButton>(play_layer, 0, true);
                orig<&Hooks::PlayLayer_pushButton>(play_layer, 0, true);
            }
            if (play_layer->m_levelSettings->m_twoPlayerMode)
                record_action(false, false, false);
            practice_fixes.apply_checkpoint();
        }
    }
}

void ReplaySystem::handle_playing() {
    if (is_playing()) {
        auto x = gd::GameManager::sharedState()->getPlayLayer()->m_player1->m_position.x;
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
                if (recorder.m_recording && (!recorder.m_until_end || from_offset<bool>(play_layer, 0x4BD)))
                    recorder.stop();
                break;
            case RECORDING:
                label->setString("Recording");
                break;
            case PLAYING:
                label->setString(showcase_mode ? "" : "Playing");
                break;
        }
    } else if (recorder.m_recording) {
        recorder.stop();
    }
}

std::filesystem::path get_save_file_path() {
    const auto path = CCFileUtils::sharedFileUtils()->getWritablePath();
    return std::filesystem::path(path) / "replaybot.txt";
}

void ReplaySystem::save() {
    const auto path = get_save_file_path();
    std::ofstream file(path);
    // incredible
    file << "fps=" << default_fps << '\n';
    file << "type=" << static_cast<int>(default_type) << '\n';
    file << "real_time=" << real_time_mode << '\n';
    file << "showcase_mode=" << showcase_mode << '\n';
    file << "recorder_width=" << recorder.m_width << '\n';
    file << "recorder_height=" << recorder.m_height << '\n';
    file << "recorder_fps=" << recorder.m_fps << '\n';
    file << "recorder_until_end=" << recorder.m_until_end << '\n';
    file << "recorder_codec=" << recorder.m_codec << '\n';
    file << "recorder_bitrate=" << recorder.m_bitrate << '\n';
    file << "recorder_extra_args=" << recorder.m_extra_args << '\n';
    file << "recorder_extra_audio_args=" << recorder.m_extra_audio_args << '\n';
    file << "recorder_after_end_duration=" << recorder.m_after_end_duration << '\n';
}

void ReplaySystem::load() {
    const auto path = get_save_file_path();
    if (!std::filesystem::exists(path)) return;
    std::ifstream file(path);
    std::string line;
    // incredible
    try {
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            const auto [key, value] = split_once(line, '=');
            using namespace std::literals::string_view_literals;
            // incredible
            if (key == "fps"sv)
                default_fps = std::stof(std::string(value));
            else if (key == "type"sv)
                default_type = ReplayType(std::stoi(std::string(value)));
            else if (key == "real_time"sv)
                real_time_mode = value == "1"sv;
            else if (key == "showcase_mode"sv)
                showcase_mode = value == "1"sv;
            else if (key == "recorder_width"sv)
                recorder.m_width = std::stoul(std::string(value));
            else if (key == "recorder_height"sv)
                recorder.m_height = std::stoul(std::string(value));
            else if (key == "recorder_fps"sv)
                recorder.m_fps = std::stoul(std::string(value));
            else if (key == "recorder_until_end"sv)
                recorder.m_until_end = value == "1"sv;
            else if (key == "recorder_codec"sv)
                recorder.m_codec = value;
            else if (key == "recorder_bitrate"sv)
                recorder.m_bitrate = value;
            else if (key == "recorder_extra_args"sv)
                recorder.m_extra_args = value;
            else if (key == "recorder_extra_audio_args"sv)
                recorder.m_extra_audio_args = value;
            else if (key == "recorder_after_end_duration"sv)
                recorder.m_after_end_duration = std::stof(std::string(value));
        }
    } catch (...) {
        // no care
    }

}