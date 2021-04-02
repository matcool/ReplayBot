#include "replay.hpp"

void Replay::remove_actions_after(float x) {
	auto check = [&](const Action& action) -> bool {
		return action.x >= x;
	};
	actions.erase(std::remove_if(actions.begin(), actions.end(), check), actions.end());
}

void Replay::save(const std::string& path) {
	// lol
}

Replay Replay::load(const std::string& path)  {
	Replay replay(60);
	// bla bla do some stuff
	return std::move(replay);
}