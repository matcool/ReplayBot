#pragma once
#include "pch.h"

namespace PlayerObject {
	void setup(uintptr_t base);

	inline void(__thiscall* pushButton)(void* self, void*);
	void __fastcall pushButtonHook(void* self, void*, void*);

	inline void(__thiscall* releaseButton)(void* self, void*);
	void __fastcall releaseButtonHook(void* self, void*, void*);
}

