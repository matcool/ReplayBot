#pragma once
#include "pch.h"

using namespace cocos2d;

namespace PauseLayer {
	void setup(uintptr_t);

	inline void(__thiscall* init)(CCLayer* self);
	void __fastcall initHook(CCLayer* self, void*);

	class Callbacks {
	public:
		void recordBtn(CCObject*);
		void playBtn(CCObject*);
		void loadBtn(CCObject*);
		void saveBtn(CCObject*);
	};
}
