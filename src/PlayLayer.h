#pragma once
#include "includes.h"
#include "PlayerObject.h"

namespace PlayLayer {
	void setup(uintptr_t base);
	
	constexpr int StatusLabelTag = 666;
	inline CCLayer* self;
	inline uintptr_t base;

	inline bool(__thiscall* init)(CCLayer* self, void*);
	bool __fastcall initHook(CCLayer* self, void*, void*);

	void updateStatusLabel(const char* text);

	inline void(__thiscall* update)(CCLayer* self, float dt);
	void __fastcall updateHook(CCLayer* self, void*, float dt);

	inline void(__thiscall* schUpdate)(CCScheduler* self, float dt);
	void __fastcall schUpdateHook(CCScheduler* self, void*, float dt);

	inline void*(__thiscall* levelComplete)(CCLayer* self);
	void* __fastcall levelCompleteHook(CCLayer* self, void*);

	inline void* (__thiscall* onQuit)(CCLayer* self);
	void* __fastcall onQuitHook(CCLayer* self, void*);

	inline void* (__thiscall* onEditor)(CCLayer* self, void*);
	void* __fastcall onEditorHook(CCLayer* self, void*, void*);

	inline int (__thiscall* resetLevel)(CCLayer* self);
	int __fastcall resetLevelHook(CCLayer* self, void*);

	inline int(__thiscall* pushButton)(CCLayer* self, int, bool);
	int __fastcall pushButtonHook(CCLayer* self, void*, int, bool);

	inline int(__thiscall* releaseButton)(CCLayer* self, int, bool);
	int __fastcall releaseButtonHook(CCLayer* self, void*, int, bool);

	inline void*(__thiscall* markCheckpoint)(CCLayer* self, void*);
	void* __fastcall markCheckpointHook(CCLayer* self, void*, void*);

	inline void*(__fastcall* removeLastCheckpoint)(CCLayer* self);
	void* __fastcall removeLastCheckpointHook(CCLayer* self, void*);

	bool is2Player();

	PlayerObject* getPlayer();
	PlayerObject* getPlayer2();
	uintptr_t getSelf();
};

