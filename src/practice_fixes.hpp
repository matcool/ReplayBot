#pragma once
#include <stack>
#include "includes.h"

struct CheckpointData {
    double y_accel;
    float rotation;
    bool buffer_orb;
	static CheckpointData from_player(gd::PlayerObject* player) {
		return { player->m_yAccel, player->getRotation(), player->m_hasJustHeld };
	};
	void apply(gd::PlayerObject* player) {
		player->m_yAccel = y_accel;
		player->setRotation(rotation);
	}
};

struct Checkpoint {
    CheckpointData player1;
    CheckpointData player2;
    size_t activated_objects_size;
    size_t activated_objects_p2_size;
    unsigned frame;
};

class PracticeFixes {
    std::stack<Checkpoint> checkpoints;
    std::vector<gd::GameObject*> activated_objects;
    std::vector<gd::GameObject*> activated_objects_p2;
    friend class ReplaySystem;
public:
	void add_checkpoint(unsigned frame = 0) {
        auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
        checkpoints.push({
            CheckpointData::from_player(play_layer->m_player1),
            CheckpointData::from_player(play_layer->m_player2),
            activated_objects.size(),
            activated_objects_p2.size(),
            frame
        });
    }
    
	void remove_checkpoint() {
        if (!checkpoints.empty()) {
            checkpoints.pop();
            if (checkpoints.empty()) {
                activated_objects.clear();
            } else {
                activated_objects.erase(activated_objects.begin() + checkpoints.top().activated_objects_size, activated_objects.end());
            }
        }
    }

    void apply_checkpoint() {
        if (!checkpoints.empty()) {
            auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
            auto checkpoint = checkpoints.top();
            checkpoint.player1.apply(play_layer->m_player1);
            checkpoint.player2.apply(play_layer->m_player2);
        }
    }
    
	void clear_checkpoints() {
        while (!checkpoints.empty())
            checkpoints.pop();
    }

    void add_activated_object(gd::GameObject* object) {
        activated_objects.push_back(object);
    }
    void add_activated_object_p2(gd::GameObject* object) {
        activated_objects_p2.push_back(object);
    }
};