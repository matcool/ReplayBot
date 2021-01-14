#pragma once
#include <vector>

struct Action {
	float x;
	bool hold;
	bool player2;
};

class Replay {
	std::vector<Action> actions;
	float fps;
public:
	Replay(float fps);

	float getFPS() {
		return fps;
	}

	void addAction(Action);
	void removeActionsAfterX(float x);

	std::vector<Action> getActions() {
		return actions;
	}
};
