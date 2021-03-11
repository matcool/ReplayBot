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
		if (!showcaseMode)
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

void ReplaySystem::onReset() {
	if (recording) {
		auto x = PlayerObject::getX(PlayLayer::getPlayer());
		currentReplay->removeActionsAfterX(*x);
		recordAction(PracticeFixes::isHolding, true);
		// you cant "buffer hold" player 2
		if (PlayLayer::is2Player())
			recordAction(false, false);
		PracticeFixes::applyCheckpoint();
	}
	else if (playing) {
		PlayLayer::releaseButton(PlayLayer::self, 0, true);
		PlayLayer::releaseButton(PlayLayer::self, 0, false);
		curActionIndex = 0;
	}
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
	currentReplay = Replay::load(path);
}

void ReplaySystem::saveReplay(const char* path) {
	if (currentReplay) {
		currentReplay->save(path);
	}
}

bool ReplaySystem::toggleShowcaseMode() {
	return (showcaseMode = !showcaseMode);
}
