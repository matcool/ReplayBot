#include "GameManager.h"

void GameManager::setup(uintptr_t base) {
	// from https://github.com/AndreNIH/gdduck/blob/master/GameManager.cpp
	getSharedState = reinterpret_cast<decltype(getSharedState)>(base + 0xC4A50);
	getGameVariable = reinterpret_cast<decltype(getGameVariable)>(base + 0xC9D30);
}

bool GameManager::is2PFlipped() {
	return getGameVariable(getSharedState(), "0010");
}