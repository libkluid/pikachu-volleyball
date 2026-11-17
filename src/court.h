#pragma once
#include "graphics.h"

// ---------------------------------------------------------------------------
//  CCourt
//  Owns the background sheet, the 10 score digit sprites, the shadow sprite,
//  the cloud sprites and the animated cloud/wave state.
// ---------------------------------------------------------------------------
class CCourt {
public:
    CCourt();
    ~CCourt();

    bool init(CScreen* screen1, CScreen* screen2);
    void drawBackground(CScreen* screen);
    void updateAnimation(CScreen* screen);
    void drawScore(CScreen* screen, int score1, int score2);

    CScreen* m_pScreen1 = nullptr;
    CScreen* m_pScreen2 = nullptr;
    CSpriteSheet* m_pSheet = nullptr;  // BMP_COURT
    CSprite* m_pCloudSprite = nullptr;  // (0,16,48,24)
    CSprite* m_pWaveSprite = nullptr;  // (104,0,16,32)
    CSpriteSheet* m_pDigits = nullptr;  // BMP_DIGITS
    CSprite* m_pDigit[10] = {};
    CRenderQueue* m_pQueue = nullptr;

    int m_cloudX[10] = {};
    int m_cloudY[10] = {};
    int m_cloudSpeed[10] = {};
    int m_cloudFrame[10] = {};
    int m_waveOffset = 0;
    int m_waveDirection = 2;
};
