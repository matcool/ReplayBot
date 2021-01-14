#include "ReplaySystem.h"

void ReplaySystem::init() {
	baseAddress = reinterpret_cast<uintptr_t>(GetModuleHandle(0));
}