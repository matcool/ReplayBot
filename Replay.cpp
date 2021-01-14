#include "Replay.h"
#include <iostream>

Replay::Replay(float fps) : fps(fps) {}

void Replay::addAction(Action action) {
	// std::cout << "Action: hold=" << action.hold << " player2=" << action.player2 << " x=" << action.x << std::endl;
	actions.push_back(action);
}

void Replay::removeActionsAfterX(float x) {
	auto check = [&](Action action) -> bool {
		return action.x > x;
	};
	actions.erase(std::remove_if(actions.begin(), actions.end(), check), actions.end());
}
