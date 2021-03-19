#pragma once
#include "includes.h"

namespace PauseLayer {
	void setup(uintptr_t);

	inline bool(__thiscall* init)(CCLayer* self);
	bool __fastcall initHook(CCLayer* self, void*);

	inline void(__thiscall* onPause)(void* self, void*);
	void __fastcall onPauseHook(void* self, void*, void* idk);
}
