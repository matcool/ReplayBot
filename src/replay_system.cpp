#include "replay_system.hpp"
#include "hooks.hpp"

void ReplaySystem::record_action(bool hold, bool player1, bool flip) {
    if (is_recording()) {
        auto gm = gd::GameManager::sharedState();
        auto play_layer = gm->getPlayLayer();
        auto is_two_player = play_layer->m_levelSettings->m_twoPlayerMode;
        player1 ^= flip && gm->getGameVariable("0010");
        get_replay().add_action({ play_layer->m_player1->position.x, hold, is_two_player && !player1 });
    }
}

void ReplaySystem::play_action(const Action& action) {
    auto gm = gd::GameManager::sharedState();
    auto flip = gm->getGameVariable("0010");
    auto func = action.hold ? Hooks::PlayLayer::pushButton : Hooks::PlayLayer::releaseButton;
    func(gm->getPlayLayer(), 0, !action.player2 ^ flip);
}

void ReplaySystem::on_reset() {
    auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
    if (is_playing()) {
        Hooks::PlayLayer::releaseButton(play_layer, 0, false);
        Hooks::PlayLayer::releaseButton(play_layer, 0, true);
        action_index = 0;
    } else if (is_recording()) {
        replay.remove_actions_after(play_layer->m_player1->position.x);
        auto& activated_objects = practice_fixes.activated_objects;
        if (practice_fixes.checkpoints.empty()) {
            activated_objects.clear();
        } else {
            activated_objects.erase(
                activated_objects.begin() + practice_fixes.checkpoints.top().activated_objects_size,
                activated_objects.end()
            );
            for (const auto object : activated_objects) {
                object->m_hasBeenActivated = true;
            }
        }
        const auto& actions = replay.get_actions();
        bool holding = play_layer->m_player1->isActuallyHolding;
        if ((holding && actions.empty()) || (!actions.empty() && actions.back().hold != holding)) {
            record_action(holding, true, false);
            if (holding) {
                Hooks::PlayLayer::releaseButton(play_layer, 0, true);
                Hooks::PlayLayer::pushButton(play_layer, 0, true);
                play_layer->m_player1->canBufferOrb = true;
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