#pragma once
#include "graphics.h"
#include "audio.h"

class CPlayer;

// ---------------------------------------------------------------------------
//  CBall
// ---------------------------------------------------------------------------
class CBall {
public:
    CBall();
    ~CBall();

    bool init(CScreen* screen1, CScreen* screen2);
    void setState(const int* state);
    void getState(int* out) const;
    void predictLanding();
    bool updateWorld();
    void draw(CScreen* screen);
    void clearQueue();
    void getRect(RECT* out) const;
    bool hitTest(int px, int py) const;
    void hitByPlayer(int playerX, const CUserInput& input, int playerState);

    CSpriteSheet* m_pSheet = nullptr;
    CSprite* m_pFrame[5] = {};
    CSprite* m_pEffect1 = nullptr;
    CSprite* m_pEffect2 = nullptr;
    CSprite* m_pPunch = nullptr;
    CSound* m_pSoundHit = nullptr;
    CSound* m_pSoundGround = nullptr;
    CRenderQueue* m_pQueue = nullptr;

    int x = 56;
    int y = 0;
    int vx = 0;
    int vy = 1;
    int expectedX = 0;
    int rotation = 0;
    int fineRotation = 0;
    int punchRadius = 0;
    int punchX = 0;
    int punchY = 0;
    int previousX = 0;
    int previousPreviousX = 0;
    int previousY = 0;
    int previousPreviousY = 0;
    bool isPowerHit = false;

    // initial values for a new round
    void initForNewRound(bool player2Serves);
};
