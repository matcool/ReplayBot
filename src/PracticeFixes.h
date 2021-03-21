#pragma once
#include <stack>
#include "PlayerObject.h"

struct CheckpointData {
	double yAccel;
	float rotation;
	static CheckpointData fromPlayer(PlayerObject* player) {
		return { player->m_yAccel, player->getRotation() };
	};
	void apply(PlayerObject* player) {
		player->m_yAccel = yAccel;
		player->setRotation(rotation);
	}
};

struct Checkpoint {
	CheckpointData player1;
	CheckpointData player2;
};

namespace PracticeFixes {
	inline std::stack<Checkpoint> checkpoints;
	// TODONT: 2 player
	// nvm lmao gd itself doesnt do it
	inline bool isHolding = false;

	void applyCheckpoint();

	void addCheckpoint();
	void removeCheckpoint();
	Checkpoint currentCheckpoint();
	void clearCheckpoints();
}