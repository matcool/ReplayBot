#ifndef PCH_H
#define PCH_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

// shut up cocos
#pragma warning(push, 0)
#include <cocos2d.h>
using namespace cocos2d;
#pragma warning(pop)

#include <MinHook.h>
#include <gd.h>

#endif //PCH_H
