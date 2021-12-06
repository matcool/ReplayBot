#pragma once
#include "includes.h"
#include <vector>
#include <matdash.hpp>

namespace Hooks {
    void init();

    void PlayLayer_pushButton(gd::PlayLayer*, int, bool);
    void PlayLayer_releaseButton(gd::PlayLayer*, int, bool);
}