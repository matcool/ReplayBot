#pragma once
#include "pch.h"

using namespace cocos2d;

namespace MenuLayer {
	inline bool(__thiscall* init)(CCLayer* self);
	bool __fastcall initHook(CCLayer* self);

	void memInit(uintptr_t base, HMODULE, HANDLE);
}

class UnloadBtn {
public:
	void callback(CCObject*);
	inline static HMODULE hModule;
	inline static HANDLE hThread;
};