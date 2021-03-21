#pragma once
#include "includes.h"

class PlayerObject : public CCSprite {
public:
	// sizeof(CCSprite) = 484
	char _pad0[0x444];
	double m_yAccel; // + 0x628
	char _pad1[0x4C];
	float m_xPos; // + 0x67C
	float m_yPos;
};

namespace PlayerObjectHooks {
	void setup(uintptr_t base);

	inline bool preventInput = false;

	inline void(__thiscall* releaseButton)(PlayerObject* self, void*);
	void __fastcall releaseButtonHook(PlayerObject* self, void*, void*);
};