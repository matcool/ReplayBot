#pragma once
#include <stack>
#include "PlayerObject.h"

struct CheckpointData {
	double yAccel;
	float rotation;
	float spriteRotation;
	static CheckpointData fromPlayer(uintptr_t player) {
		return { *PlayerObject::getYAccel(player), *PlayerObject::getRotation(player), *PlayerObject::getSpriteRotation(player) };
	};
	void apply(uintptr_t player) {
		*PlayerObject::getYAccel(player) = yAccel;
		*PlayerObject::getRotation(player) = rotation;
		*PlayerObject::getSpriteRotation(player) = spriteRotation;
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