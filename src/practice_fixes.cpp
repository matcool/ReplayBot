#include "practice_fixes.hpp"
#include "replay_system.hpp"

void CheckpointObjectMod::saveInfo() {
    auto& rs = ReplaySystem::get_instance();
    m_checkpoint = rs.get_practice_fixes().create_checkpoint();
}

Checkpoint PracticeFixes::create_checkpoint() {
    auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
    if (play_layer) {
        auto& rs = ReplaySystem::get_instance();
        return {
            CheckpointData::from_player(play_layer->m_player1),
            CheckpointData::from_player(play_layer->m_player2),
            activated_objects.size(),
            activated_objects_p2.size(),
            rs.get_frame()
        };
    } else {
        return {};
    }
}