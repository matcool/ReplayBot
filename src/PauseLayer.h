#pragma once
#include "pch.h"

using namespace cocos2d;

namespace PauseLayer {
	void setup(uintptr_t);
	void unload(uintptr_t);

	inline void(__thiscall* init)(CCLayer* self);
	void __fastcall initHook(CCLayer* self, void*);

	inline void(__thiscall* onPause)(void* self, void*);
	void __fastcall onPauseHook(void* self, void*, void* idk);

	class Callbacks {
	public:
		void recordBtn(CCObject*);
		void playBtn(CCObject*);
		void loadBtn(CCObject*);
		void saveBtn(CCObject*);
	};
}
