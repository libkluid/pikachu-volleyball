#pragma once
#include "graphics.h"
#include "audio.h"
#include "pika.h"

class CBall;

// ---------------------------------------------------------------------------
//  CPlayer
//
//  Layout:
//    sprite sheet (BMP_PLAYER)
//    35 sprite objects (7 rows x 5), only 28 are actually used
//    CSound chu      (jump/dive)
//    CSound pika     (power hit)
//    CSound pipikachu(win)
//    render queue
//    isPlayer2
//    isComputer
//    x, y, yVelocity
//    divingDirection, lyingDownTimer, ballHitLatch
//    state, frameNumber, armDirection, delay
//    isWinner, gameEnded, computerBoldness
// ---------------------------------------------------------------------------
class CPlayer {
public:
    enum State {
        Normal = 0,
        Jumping = 1,
        PowerHitJumping = 2,
        Diving = 3,
        LyingDown = 4,
        Won = 5,
        Lost = 6,
    };

    CPlayer();
    ~CPlayer();

    bool init(CScreen* screen1, CScreen* screen2, int side);
    bool setSide(int side);
    bool initRound();
    void setGameEnd(bool winner);

    void update(const CUserInput& input, int otherPlayerX, const int* ballState);
    void runAI(int otherPlayerX, const int* ballState, CUserInput& input);
    bool decidePowerHit(int otherPlayerX, const int* ballState, CUserInput& input);
    int expectedWhenPowerHit(int xDirection, int yDirection, const int* ballState) const;
    void updateWinLoseFrame();

    void draw(CScreen* screen);
    void getPosition(int* out) const { out[0] = x; out[1] = y; }
    void getRect(RECT* out) const;
    void clearQueue();

    CSpriteSheet* m_pSheet = nullptr;
    CSprite* m_pSprite[35] = {};
    CSound* m_pSoundChu = nullptr;
    CSound* m_pSoundPika = nullptr;
    CSound* m_pSoundPipi = nullptr;
    CRenderQueue* m_pQueue = nullptr;

    int isPlayer2 = 0;
    int isComputer = 0;
    int x = 36;
    int y = PLAYER_GROUND_Y;
    int yVelocity = 0;
    int divingDirection = 0;
    int lyingDownTimer = -1;
    int ballHitLatch = 0;
    int state = Normal;
    int frameNumber = 0;
    int armDirection = 1;
    int delayBeforeNextFrame = 0;
    int isWinner = 0;
    int gameEnded = 0;
    int computerBoldness = 0;
    int computerWhereToStandBy = 0;         // extra field
};
