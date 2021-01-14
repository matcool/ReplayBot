#pragma once
#include "pch.h"

uintptr_t follow(uintptr_t address) {
    return *(reinterpret_cast<uintptr_t*>(address));
}