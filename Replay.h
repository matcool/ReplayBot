#pragma once
#include <vector>

struct Action {
	float x;
	bool hold;
};

using namespace cocos2d;

namespace Replay {
	inline bool(__thiscall* pushButton)(void* self, void*);
	bool __fastcall pushButtonHook(void* self, void*, void*);

	inline bool(__thiscall* releaseButton)(void* self, void*);
	bool __fastcall releaseButtonHook(void* self, void*, void*);

	inline void(__thiscall* playLayerUpdate)(cocos2d::CCLayer* self, float);
	void __fastcall playLayerUpdateHook(cocos2d::CCLayer* self, void*, float);

	inline void(__thiscall* playLayerInit)(cocos2d::CCLayer* self, void*);
	void __fastcall playLayerInitHook(cocos2d::CCLayer* self, void*, void*);

	inline void(__thiscall* pauseLayerInit)(cocos2d::CCLayer* self);
	void __fastcall pauseLayerInitHook(cocos2d::CCLayer* self, void*);

	void toggleRecording();
	void togglePlaying();

	void memInit(uintptr_t base);
	inline std::vector<Action> actions;
	inline bool recording;
	inline bool playing;
	inline CCLayer* playLayer;
	inline float lastPlayerX = 0.f;
	inline int curPlayingAction = 0;
	inline uintptr_t player;
	inline uintptr_t player2;
};

class Callbacks {
public:
	void recordBtnCallback(CCObject*);
	void playBtnCallback(CCObject*);
};