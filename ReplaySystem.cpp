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
		currentReplay = std::make_shared<Replay>(120.f);
	}
}

void ReplaySystem::togglePlaying() {
	std::cout << "Toggled Playing" << std::endl;
	playing = !playing;
	recording = false;
	PlayLayer::updateStatusLabel(playing ? "Playing" : "");
	if (playing) {
        curActionIndex = 0;
	}
}

void ReplaySystem::handleRecording() {
	auto x = reinterpret_cast<float*>(PlayLayer::getPlayer() + 0x67C);
	if (*x < lastPlayerX) {
		std::cout << "Player died at " << *x << std::endl;
		currentReplay->removeActionsAfterX(*x);
	}
	lastPlayerX = *x;
}

void playAction(Action action) {
	// std::cout << "played: " << action.hold << " " << action.player2 << " " << action.x << std::endl;
	auto player = reinterpret_cast<void*>(action.player2 ? PlayLayer::getPlayer2() : PlayLayer::getPlayer());
	if (action.hold) PlayerObject::pushButton(player, 0);
	else PlayerObject::releaseButton(player, 0);
}

void ReplaySystem::handlePlaying() {
	auto x = reinterpret_cast<float*>(PlayLayer::getPlayer() + 0x67C);
	auto actions = currentReplay->getActions();
	if (curActionIndex < actions.size()) {
		auto curAction = actions[curActionIndex];
		if (*x >= curAction.x) {
			*x = curAction.x;
			playAction(curAction);
			curActionIndex++;
			// kinda hacky but whatever
			// TODO: why arent duals accurate???
			if (curActionIndex < actions.size()) {
				auto nextAction = actions[curActionIndex];
				if (nextAction.x == curAction.x) {
					playAction(nextAction);
					curActionIndex++;
				}
			}
		}
	}
	else {
		togglePlaying();
	}
}