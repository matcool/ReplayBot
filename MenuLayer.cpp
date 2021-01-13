#include "MenuLayer.h"

bool __fastcall MenuLayer::initHook(CCLayer* self) {
	auto ret = init(self);

    auto sprite = CCSprite::createWithSpriteFrameName("GJ_cancelDownloadBtn_001.png");
    auto btn = CCMenuItemSpriteExtra::create(sprite, sprite, self, nullptr);

    auto children = self->getChildren();
    CCMenu* bottomMenu = (CCMenu*)children->objectAtIndex(3);

    bottomMenu->addChild(btn, 500);

    CCMenuItem* chestBtn = (CCMenuItem*)(bottomMenu->getChildren()->objectAtIndex(4));
    bottomMenu->removeChild(chestBtn, false);
    bottomMenu->alignItemsHorizontallyWithPadding(5.0f);
    bottomMenu->addChild(chestBtn);

    return ret;
}

// the crash button
void UnloadBtn::callback(CCObject* pSender) {
    fclose(stdout);
    FreeConsole();
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    FreeLibrary(hModule);
    SuspendThread(hThread);
    //TerminateThread(hThread, 0);
}

void MenuLayer::memInit(uintptr_t base, HMODULE hModule, HANDLE hThread) {
    UnloadBtn::hModule = hModule;
    UnloadBtn::hThread = hThread;
    MH_CreateHook(
        reinterpret_cast<void*>(base + 0x1907B0),
        MenuLayer::initHook,
        reinterpret_cast<void**>(&MenuLayer::init)
    );
}