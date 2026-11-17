#pragma once
#include "graphics.h"
#include "audio.h"
#include "pika.h"

// ---------------------------------------------------------------------------
//  CTitleScene
//
//  Mode bits returned to CGame:
//    1 -> 1P (left human, right CPU)
//    2 -> 1P (left CPU, right human)
//    4 -> 2P
//    8 -> CPU vs CPU demo
// ---------------------------------------------------------------------------
class CTitleScene {
public:
    CTitleScene();
    ~CTitleScene();

    bool init(CScreen* screen1, CScreen* screen2);
    void reset();
    int update(const CUserInput* inputs);

    void drawScrollingBackground(CScreen* screen);
    void drawLogo(CScreen* screen);
    void drawBigLogo(CScreen* screen);
    void drawMenu(CScreen* screen);

    CScreen* m_pScreen1 = nullptr;
    CScreen* m_pScreen2 = nullptr;
    CSpriteSheet* m_pLogoSheet = nullptr;  // (0x77)
    CSprite* m_pLogoSprite = nullptr;
    CSpriteSheet* m_pTextSheet = nullptr;  // (0x73)
    CSprite* m_pTextSprite = nullptr;
    CSpriteSheet* m_pMegaSheet = nullptr;  // (0x71)
    CSprite* m_pMegaSprite = nullptr;
    CSpriteSheet* m_pControlSheet = nullptr;  // (0x7D)
    CSprite* m_pControlSprite[2] = {};  // (two menu lines)
    CSpriteSheet* m_pFaceSheet = nullptr;  // (0x78)
    CSpriteSheet* m_pArtSheet = nullptr;  // (0x79)
    CSprite* m_pArtSprite = nullptr;        // intro art (0x79)
    CSpriteSheet* m_pCopyrightSheet = nullptr;  // (0x7E)
    CSprite* m_pCopyrightSprite = nullptr;
    CSound* m_pSound1 = nullptr;  // (0x90)
    CSound* m_pSound2 = nullptr;  // (0x8F)

    int m_phase = 0;  // (0 intro, 1 menu)
    int m_menuPhase = 0;  // (0..6 logo/menu animation)
    int m_bigTimer = 0;
    int m_logoTimer = 0;
    int m_scroll = 0;
    int m_introTimer = 0;
    int m_selection = 0;  // (0 = 1P, 1 = 2P)
    int m_animTimer = 0;
    int m_result = 0;
    int m_idleTimer = 0;

    // Edge state: a press that leaves the intro must not leak into the title.
    bool m_powerLatch = true;
};
