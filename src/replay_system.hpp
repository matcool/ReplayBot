#pragma once
#include "replay.hpp"
#include "practice_fixes.hpp"

// this is a rly bad name
enum RSState {
    NOTHING,
    RECORDING,
    PLAYING
};

class ReplaySystem {
    static ReplaySystem* instance;

    float default_fps;

    Replay replay;
    RSState state = NOTHING;

    size_t action_index = 0;

    PracticeFixes practice_fixes;

    bool frame_advance = false;

    ReplaySystem() : default_fps(120.f), replay(default_fps) {}
public:
    static ReplaySystem* get_instance() {
        if (!instance) instance = new ReplaySystem();
		return instance;
    }

    inline auto& get_replay() { return replay; }
    inline auto get_default_fps() { return default_fps; }
    inline void set_default_fps(float fps) { default_fps = fps; }

    inline bool is_playing() { return state == PLAYING; }
    inline bool is_recording() { return state == RECORDING; }

    inline void toggle_playing() { state = is_playing() ? NOTHING : PLAYING; }
    inline void toggle_recording() {
        state = is_recording() ? NOTHING : RECORDING;
        if (!is_recording()) frame_advance = false;
    }

    inline void reset_state() {
        state = NOTHING;
        frame_advance = false;
    } 

    void record_action(bool hold, bool player1, bool flip = true);
    void play_action(const Action& action);

    void on_reset();

    void handle_playing();

    inline auto& get_practice_fixes() { return practice_fixes; }

    inline bool get_frame_advance() { return frame_advance; }
    inline void set_frame_advance(bool b) { frame_advance = b; }
};