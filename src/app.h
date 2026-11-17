#pragma once
#include "pika.h"
#include "game.h"
#include "input.h"
#include "audio.h"
#include "resource_ids.h"

// ---------------------------------------------------------------------------
//  CApp
// ---------------------------------------------------------------------------
class CApp {
public:
    CApp();
    ~CApp();

    bool init(HINSTANCE hInstance, int nCmdShow);
    int run();
    bool update();
    void drawFrame();
    void paint(HDC hdc);
    bool onCommand(UINT id);
    void onKeyDown(UINT vk);
    void setPaused(bool paused);
    void snapshotFps();
    void startDemo();

    CTitleScene* titleScene() { return m_pGame ? m_pGame->m_pTitle : nullptr; }
    bool isPaused() const { return m_paused; }
    CGame* game() { return m_pGame; }

private:
    bool createWindow(HINSTANCE hInstance, int nCmdShow);
    void decodeInput(unsigned int mask, CUserInput& out);

    HWND m_hwnd = nullptr;
    HINSTANCE m_hInstance = nullptr;
    CDirectSoundMgr* m_pSoundMgr = nullptr;
    CPlayerInput* m_pInput[2] = {};
    CGame* m_pGame = nullptr;
    CUserInput m_inputs[2];
    int m_intervalMs = 40;
    DWORD m_lastUpdate = 0;
    bool m_paused = false;
    UINT m_speedMenuId = IDM_SPEED_NORM;
    UINT m_scoreMenuId = IDM_SCORE_15;
    bool m_debug = false;
    int m_frameCounter = 0;
    int m_fps = 0;
};
