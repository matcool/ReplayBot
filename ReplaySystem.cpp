#include "ReplaySystem.h"
#include "PlayLayer.h"
#include "PracticeFixes.h"
#include "GameManager.h"

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

void ReplaySystem::recordAction(bool hold, bool player1) {
	if (recording) {
		auto x = reinterpret_cast<float*>(PlayLayer::getPlayer() + 0x67c);
		currentReplay->addAction({ *x, hold, PlayLayer::is2Player() && !player1 });
	}
}

void ReplaySystem::handleRecording() {
	auto x = reinterpret_cast<float*>(PlayLayer::getPlayer() + 0x67C);
	if (*x < lastPlayerX) {
		std::cout << "Player died at " << lastPlayerX << " " << PracticeFixes::isHolding << std::endl;
		currentReplay->removeActionsAfterX(*x);
		recordAction(PracticeFixes::isHolding, true);
		// you cant "buffer hold" player 2
		if (PlayLayer::is2Player())
			recordAction(false, false);
		PracticeFixes::applyCheckpoint();
	}
	lastPlayerX = *x;
}

void ReplaySystem::playAction(Action action) {
	auto layer = PlayLayer::self;
	// if (!PlayLayer::is2Player() && action.player2) return;
	auto flip = PlayLayer::is2Player() && GameManager::is2PFlipped();
	if (action.hold) PlayLayer::pushButton(layer, 0, !action.player2 ^ flip);
	else PlayLayer::releaseButton(layer, 0, !action.player2 ^ flip);
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