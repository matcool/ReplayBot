#pragma once
#include "pch.h"

namespace GameManager {
	void setup(uintptr_t);
	
	inline void* (__cdecl* getSharedState)();
	inline bool(__thiscall* getGameVariable)(void* self, const char* key);

	bool is2PFlipped();
}
