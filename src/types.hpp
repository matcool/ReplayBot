#pragma once
#include "includes.h"

class PlayerObject : public gd::GameObject {
	// sizeof(GameObject) = 0x468
protected:
	PAD(0x1C0);
public:
	double y_accel; // + 0x628
protected:
	PAD(0x4C);
public:
	float x_pos; // + 0x67C
	float y_pos;
};

class LevelSettingsObject : public CCNode {
protected:
    PAD(14);
    bool two_player_mode;
public:
    bool is_two_player() { return two_player_mode; }
};

class PlayLayer : public CCLayer {
    PAD(0x108);
    PlayerObject* player;
    PlayerObject* player2;

    LevelSettingsObject* level_settings;

    PAD(0x16c);

    bool player_dead;
public:
    auto get_player() { return player; }
    auto get_player2() { return player2; }

    auto get_level_settings() { return level_settings; }

    auto is_player_dead() { return player_dead; }
};