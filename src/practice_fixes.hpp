#pragma once
#include <stack>
#include "includes.h"

struct CheckpointData {
    double y_accel;
    float rotation;
    bool buffer_orb;
	static CheckpointData from_player(gd::PlayerObject* player) {
		return { player->m_yAccel, player->getRotation(), player->m_hasJustHeld };
	};
	void apply(gd::PlayerObject* player) const {
		player->m_yAccel = y_accel;
		player->setRotation(rotation);
	}
};

struct Checkpoint {
    CheckpointData player1;
    CheckpointData player2;
    size_t activated_objects_size;
    size_t activated_objects_p2_size;
    unsigned frame;
};

class CheckpointObjectMod : public gd::CheckpointObject {
public:
    Checkpoint m_checkpoint; // great name i know
    void saveInfo();
    static auto create() {
        auto ret = new CheckpointObjectMod;
        if (ret && ret->init()) {
            ret->autorelease();
            ret->saveInfo();
        } else {
            CC_SAFE_DELETE(ret);
        }
        return ret;
    }
};

class PracticeFixes {
    std::stack<Checkpoint> checkpoints;
    std::vector<gd::GameObject*> activated_objects;
    std::vector<gd::GameObject*> activated_objects_p2;
    friend class ReplaySystem;
public:
    Checkpoint create_checkpoint();

    Checkpoint get_last_checkpoint() {
        auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
        if (play_layer && play_layer->m_checkpoints->count()) {
            auto checkpoint_obj = dynamic_cast<CheckpointObjectMod*>(play_layer->m_checkpoints->lastObject());
            if (checkpoint_obj) {
                return checkpoint_obj->m_checkpoint;
            }
        }
        return {};
    }

    void apply_checkpoint() {
        auto play_layer = gd::GameManager::sharedState()->getPlayLayer();
        if (play_layer) {
            const auto checkpoint = get_last_checkpoint();
            checkpoint.player1.apply(play_layer->m_player1);
            checkpoint.player2.apply(play_layer->m_player2);
        }
    }
    
    void add_activated_object(gd::GameObject* object) {
        activated_objects.push_back(object);
    }
    void add_activated_object_p2(gd::GameObject* object) {
        activated_objects_p2.push_back(object);
    }
};