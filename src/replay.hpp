#pragma once
#include <vector>
#include <string>
#include <iostream>

struct Action {
	float x;
	bool hold;
	bool player2;
};

class Replay {
    std::vector<Action> actions;
    float fps;
public:
    Replay(float fps) : fps(fps) {}

    float get_fps() { return fps; }
    
    void add_action(const Action& action) { actions.push_back(action); }
    auto& get_actions() { return actions; }
    void remove_actions_after(float x);

    void save(const std::string& path);
    static Replay load(const std::string& path);
};