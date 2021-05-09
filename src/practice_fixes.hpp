#pragma once
#include <stack>
#include "types.hpp"
#include "includes.h"

struct CheckpointData {
	double y_accel;
	float rotation;
    bool buffer_orb;
	static CheckpointData from_player(PlayerObject* player) {
		return { player->y_accel, player->getRotation(), *cast<bool*>(cast<uintptr_t>(player) + 0x612) };
	};
	void apply(PlayerObject* player) {
		player->y_accel = y_accel;
		player->setRotation(rotation);
	}
};

struct Checkpoint {
    CheckpointData player1;
    CheckpointData player2;
    size_t activated_objects_size;
};

class PracticeFixes {
    std::stack<Checkpoint> checkpoints;
    std::vector<gd::GameObject*> activated_objects;
    friend class ReplaySystem;
public:
    PracticeFixes() {}

	void add_checkpoint() {
        auto play_layer = cast<PlayLayer*>(gd::GameManager::sharedState()->getPlayLayer());
        checkpoints.push({
            CheckpointData::from_player(play_layer->get_player()),
            CheckpointData::from_player(play_layer->get_player2()),
            activated_objects.size()
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
            auto play_layer = cast<PlayLayer*>(gd::GameManager::sharedState()->getPlayLayer());
            auto checkpoint = checkpoints.top();
            checkpoint.player1.apply(play_layer->get_player());
            checkpoint.player2.apply(play_layer->get_player2());
        }
    }
    
	void clear_checkpoints() {
        while (!checkpoints.empty())
            checkpoints.pop();
    }

    void add_activated_object(gd::GameObject* object) {
        activated_objects.push_back(object);
    }
};