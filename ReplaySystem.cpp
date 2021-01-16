#include "ReplaySystem.h"
#include "PlayLayer.h"
#include "PlayerObject.h"

ReplaySystem* ReplaySystem::instance;

void ReplaySystem::init(uintptr_t base) {
	baseAddress = base;
}

void ReplaySystem::toggleRecording() {
	std::cout << "Toggled Recording" << std::endl;
	recording = !recording;
	playing = false;
	PlayLayer::updateStatusLabel(recording ? "Recording" : "");
	if (recording) {
		currentReplay = std::make_shared<Replay>(defaultFPS);
	}
}

void ReplaySystem::togglePlaying() {
	if (currentReplay) {
		std::cout << "Toggled Playing" << std::endl;
		playing = !playing;
		recording = false;
		PlayLayer::updateStatusLabel(playing ? "Playing" : "");
		if (playing) {
			curActionIndex = 0;
		}
	}
}

void ReplaySystem::handleRecording() {
	auto x = reinterpret_cast<float*>(PlayLayer::getPlayer() + 0x67C);
	if (*x < lastPlayerX) {
		std::cout << "Player died at " << lastPlayerX << std::endl;
		currentReplay->removeActionsAfterX(*x);
	}
	lastPlayerX = *x;
}

void ReplaySystem::playAction(Action action) {
	// std::cout << "played: " << action.hold << " " << action.player2 << " " << action.x << std::endl;
	if (dualModeHackyFix) {
		// ignore player 2 inputs
		if (action.player2) return;
		auto player1 = reinterpret_cast<void*>(PlayLayer::getPlayer());
		auto player2 = reinterpret_cast<void*>(PlayLayer::getPlayer2());
		if (action.hold) {
			PlayerObject::pushButton(player1, 0);
			PlayerObject::pushButton(player2, 0);
		} else {
			PlayerObject::releaseButton(player1, 0);
			PlayerObject::releaseButton(player2, 0);
		}
	}
	else {
		auto player = reinterpret_cast<void*>(action.player2 ? PlayLayer::getPlayer2() : PlayLayer::getPlayer());
		if (action.hold) PlayerObject::pushButton(player, 0);
		else PlayerObject::releaseButton(player, 0);
	}
}

void ReplaySystem::handlePlaying() {
	auto x = reinterpret_cast<float*>(PlayLayer::getPlayer() + 0x67C);
	auto actions = currentReplay->getActions();
	if (curActionIndex < actions.size()) {
		Action curAction;
		// while loop since up to 4 actions can happen in the same frame
		while (curActionIndex < actions.size() && *x >= (curAction = actions[curActionIndex]).x) {
			playAction(curAction);
			++curActionIndex;
		}
	}
	else {
		togglePlaying();
	}
}

// TODO: support unicode paths

void ReplaySystem::loadReplay(const char* path) {
	currentReplay = std::make_shared<Replay>(path);
}

void ReplaySystem::saveReplay(const char* path) {
	if (currentReplay) {
		currentReplay->save(path);
	}
}