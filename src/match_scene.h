#pragma once
#include "court.h"
#include "player.h"
#include "ball.h"

// ---------------------------------------------------------------------------
//  CMatchScene
// ---------------------------------------------------------------------------
class CMatchScene {
public:
    enum State {
        StateGameStart = 0,
        StateReady = 1,
        StateRally = 2,
        StateSlowMotion = 3,
        StateGameSet = 4,
        StateQuit = 10,
    };

    CMatchScene();
    ~CMatchScene();

    bool init(CScreen* screen1, CScreen* screen2);
    // player1Computer / player2Computer are about the *players*, not the
    // sides: player 1 is the left-hand character, player 2 the right-hand one.
    bool startMatch(bool player1Computer, bool player2Computer, int maxScore);
    void resetRound(int serveSide);
    int update(const CUserInput* inputs);   // returns 1 when the scene is finished

    bool physicsStep(const CUserInput* inputs);
    void drawGameStartMessage(CScreen* screen);
    void drawReadyMessage(CScreen* screen);
    void drawGameSetMessage(CScreen* screen);

    CScreen* m_pScreen1 = nullptr;
    CScreen* m_pScreen2 = nullptr;
    CCourt* m_pCourt = nullptr;
    CPlayer* m_pPlayer[2] = {};
    CBall* m_pBall = nullptr;

    CSpriteSheet* m_pShadowSheet = nullptr;  // (BMP_SHADOW)
    CSprite* m_pShadow = nullptr;
    CSpriteSheet* m_pStartSheet = nullptr;  // (BMP_GAME_START)
    CSprite* m_pStartSprite = nullptr;
    CSpriteSheet* m_pReadySheet = nullptr;  // (BMP_READY)
    CSprite* m_pReadySprite = nullptr;
    CSpriteSheet* m_pGameSetSheet = nullptr;  // (BMP_GAME_SET)
    CSprite* m_pGameSetSprite = nullptr;
    CRenderQueue* m_pQueue = nullptr;

    int m_score[2] = { 0, 0 };
    int m_timer = 0;
    int m_state = StateGameStart;
    int m_serveSide = 0;
    int m_maxScore = 15;
};
