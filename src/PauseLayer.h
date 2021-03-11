#pragma once
#include "includes.h"

namespace PauseLayer {
	void setup(uintptr_t);

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
