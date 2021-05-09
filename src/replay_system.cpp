#include "replay_system.hpp"
#include "types.hpp"
#include "hooks.hpp"

ReplaySystem* ReplaySystem::instance;

void ReplaySystem::record_action(bool hold, bool player1, bool flip) {
    if (is_recording()) {
        auto gm = gd::GameManager::sharedState();
        auto play_layer = cast<PlayLayer*>(gm->getPlayLayer());
        auto is_two_player = play_layer->get_level_settings()->is_two_player();
        player1 ^= flip && gm->getGameVariable("0010");
        get_replay().add_action({ play_layer->get_player()->x_pos, hold, is_two_player && !player1 });
    }
}

void ReplaySystem::play_action(const Action& action) {
    auto gm = gd::GameManager::sharedState();
    auto flip = gm->getGameVariable("0010");
    auto func = action.hold ? Hooks::_PlayLayer::pushButton : Hooks::_PlayLayer::releaseButton;
    func(cast<PlayLayer*>(gm->getPlayLayer()), 0, !action.player2 ^ flip);
}

void ReplaySystem::on_reset() {
    auto play_layer = cast<PlayLayer*>(gd::GameManager::sharedState()->getPlayLayer());
    if (is_playing()) {
        Hooks::_PlayLayer::releaseButton(play_layer, 0, false);
        Hooks::_PlayLayer::releaseButton(play_layer, 0, true);
        action_index = 0;
    } else if (is_recording()) {
        replay.remove_actions_after(play_layer->get_player()->x_pos);
        auto& activated_objects = practice_fixes.activated_objects;
        if (practice_fixes.checkpoints.empty()) {
            activated_objects.clear();
        } else {
            activated_objects.erase(
                activated_objects.begin() + practice_fixes.checkpoints.top().activated_objects_size,
                activated_objects.end()
            );
            for (const auto object : activated_objects) {
                *cast<bool*>(cast<uintptr_t>(object) + 0x2ca) = true;
            }
        }
        const auto& actions = replay.get_actions();
        bool holding = *cast<bool*>(cast<uintptr_t>(play_layer->get_player()) + 0x611);
        if ((holding && actions.empty()) || (!actions.empty() && actions.back().hold != holding)) {
            record_action(holding, true, false);
            if (holding) {
                auto buffer_thing = cast<bool*>(cast<uintptr_t>(play_layer->get_player()) + 0x612);
                Hooks::_PlayLayer::releaseButton(play_layer, 0, true);
                Hooks::_PlayLayer::pushButton(play_layer, 0, true);
                *buffer_thing = true;
            }
        } else if (!actions.empty() && actions.back().hold && holding && !practice_fixes.checkpoints.empty() && practice_fixes.checkpoints.top().player1.buffer_orb) {
            Hooks::_PlayLayer::releaseButton(play_layer, 0, true);
            Hooks::_PlayLayer::pushButton(play_layer, 0, true);
        }
        if (play_layer->get_level_settings()->is_two_player())
            record_action(false, false, false);
        practice_fixes.apply_checkpoint();
    }
}

void ReplaySystem::handle_playing() {
    if (is_playing()) {
        auto x = cast<PlayLayer*>(gd::GameManager::sharedState()->getPlayLayer())->get_player()->x_pos;
        auto& actions = replay.get_actions();
        Action action;
        while (action_index < actions.size() && x >= (action = actions[action_index]).x) {
            play_action(action);
            ++action_index;
        }
        if (action_index >= actions.size())
            toggle_playing();
    }
}

// why here out of all places? idk

constexpr int STATUS_LABEL_TAG = 10032;

auto _create_status_label(PlayLayer* play_layer) {
    auto label = CCLabelBMFont::create("", "chatFont.fnt");
    label->setTag(STATUS_LABEL_TAG);
    label->setAnchorPoint({0, 0});
    label->setPosition({5, 5});
    label->setZOrder(999);
    play_layer->addChild(label);
    return label;
}

void ReplaySystem::_update_status_label() {
    auto play_layer = cast<PlayLayer*>(gd::GameManager::sharedState()->getPlayLayer());
    if (play_layer) {
        auto label = cast<CCLabelBMFont*>(play_layer->getChildByTag(10032));
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