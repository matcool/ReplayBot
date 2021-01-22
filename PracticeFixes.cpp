#include "PracticeFixes.h"
#include "PlayLayer.h"

namespace PracticeFixes {
	void applyCheckpoint() {
		if (!checkpoints.empty()) {
			auto checkpoint = currentCheckpoint();
			checkpoint.player1.apply(PlayLayer::getPlayer());
			checkpoint.player2.apply(PlayLayer::getPlayer2());
		}
	}

	void addCheckpoint() {
		checkpoints.push({ CheckpointData::fromPlayer(PlayLayer::getPlayer()), CheckpointData::fromPlayer(PlayLayer::getPlayer2()) });
	}

	void removeCheckpoint() {
		if (!checkpoints.empty())
			checkpoints.pop();
	}

	Checkpoint currentCheckpoint() {
		return checkpoints.top();
	}

	void clearCheckpoints() {
		// maybe switch to a deque? idk
		while (!checkpoints.empty())
			checkpoints.pop();
	}
}