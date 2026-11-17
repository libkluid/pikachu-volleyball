#pragma once
#include "graphics.h"
#include "audio.h"
#include "match_scene.h"
#include "title_scene.h"

// ---------------------------------------------------------------------------
//  CGame
// ---------------------------------------------------------------------------
class CGame {
public:
    CGame();
    ~CGame();

    bool init(HWND hwnd);
    void reset();
    void update(const CUserInput* inputs);

    bool playBgm(const char* filename);
    void stopBgm();

    CScreen* m_pScreen1 = nullptr;
    CScreen* m_pScreen2 = nullptr;
    CMciMusic* m_pMusic = nullptr;
    CMatchScene* m_pMatch = nullptr;
    CTitleScene* m_pTitle = nullptr;
    int m_scene = 0;  // (0 title, 1 match)
    int m_maxScore = 15;

private:
    HWND m_hwnd = nullptr;
};
