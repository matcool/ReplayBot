#pragma once
#include <stack>
#include "types.hpp"
#include "includes.h"

struct CheckpointData {
	double y_accel;
	float rotation;
	static CheckpointData from_player(PlayerObject* player) {
		return { player->y_accel, player->getRotation() };
	};
	void apply(PlayerObject* player) {
		player->y_accel = y_accel;
		player->setRotation(rotation);
	}
};

struct Checkpoint {
    CheckpointData player1;
    CheckpointData player2;
};

class PracticeFixes {
    std::stack<Checkpoint> checkpoints;
    bool holding;
public:
    PracticeFixes() {}
    bool is_holding() { return holding; }
    void set_holding(bool h) { holding = h; }

	void add_checkpoint() {
        auto play_layer = cast<PlayLayer*>(gd::GameManager::sharedState()->getPlayLayer());
        checkpoints.push({
            CheckpointData::from_player(play_layer->get_player()),
            CheckpointData::from_player(play_layer->get_player2())
        });
    }
    
	void remove_checkpoint() {
        if (!checkpoints.empty())
            checkpoints.pop();
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
};