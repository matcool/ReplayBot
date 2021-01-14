#pragma once
#include "pch.h"

using namespace cocos2d;

class ReplaySystem {
	static ReplaySystem* instance;

	uintptr_t baseAddress;
public:
	void init();

	static ReplaySystem* getInstance() {
		if (!instance) instance = new ReplaySystem();
		return instance;
	}
};

