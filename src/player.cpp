#include "player.h"
#include "ball.h"
#include "resource_ids.h"

CPlayer::CPlayer()
{
    m_pSheet = new CSpriteSheet();
    for (auto& s : m_pSprite) s = new CSprite();
    m_pSoundChu = new CSound();
    m_pSoundPika = new CSound();
    m_pSoundPipi = new CSound();
    m_pQueue = new CRenderQueue();
}

CPlayer::~CPlayer()
{
    delete m_pSheet;
    for (auto& s : m_pSprite) delete s;
    delete m_pSoundChu;
    delete m_pSoundPika;
    delete m_pSoundPipi;
    delete m_pQueue;
}

bool CPlayer::init(CScreen* screen1, CScreen* screen2, int side)
{
    if (!m_pSheet->load(BMP_PLAYER)) return false;
    m_pQueue->reset();
    (void)screen1; (void)screen2;

    // Rows 0..2: idle / jump / power-hit, 5 frames each.
    for (int row = 0; row < 3; row++)
        for (int col = 0; col < 5; col++)
            m_pSprite[row * 5 + col]->setRect(m_pSheet, col * 64, row * 64, 64, 64);

    // Row 3: dive (x=0,64) and lying (x=128,192).
    m_pSprite[15]->setRect(m_pSheet, 0, 192, 64, 64);
    m_pSprite[16]->setRect(m_pSheet, 64, 192, 64, 64);
    m_pSprite[20]->setRect(m_pSheet, 128, 192, 64, 64);
    m_pSprite[21]->setRect(m_pSheet, 192, 192, 64, 64);

    // Rows 4 and 5: win / lose, 5 frames each.
    for (int col = 0; col < 5; col++) {
        m_pSprite[25 + col]->setRect(m_pSheet, col * 64, 256, 64, 64);
        m_pSprite[30 + col]->setRect(m_pSheet, col * 64, 320, 64, 64);
    }

    m_pSoundChu->load(WAV_CHU);
    m_pSoundPika->load(WAV_PIKA);
    m_pSoundPipi->load(WAV_WIN);
    isPlayer2 = side;
    return true;
}

bool CPlayer::setSide(int side)
{
    if (side < 0 || side > 1) return false;
    isPlayer2 = side;
    isWinner = 0;
    gameEnded = 0;
    return true;
}

bool CPlayer::initRound()
{
    x = isPlayer2 ? (GROUND_WIDTH - 36) : 36;
    y = PLAYER_GROUND_Y;
    yVelocity = 0;
    divingDirection = 0;
    lyingDownTimer = -1;
    ballHitLatch = 0;
    state = Normal;
    frameNumber = 0;
    armDirection = 1;
    delayBeforeNextFrame = 0;
    computerBoldness = rand_range(5);
    computerWhereToStandBy = 0;
    return true;
}

void CPlayer::setGameEnd(bool winner)
{
    gameEnded = 1;
    isWinner = winner ? 1 : 0;
}

void CPlayer::update(const CUserInput& input, int otherPlayerX, const int* ballState)
{
    CUserInput local = input;
    if (isComputer) runAI(otherPlayerX, ballState, local);

    if (state == LyingDown) {
        int old = lyingDownTimer;
        lyingDownTimer = old - 1;
        if (old < 0) state = Normal;
        return;
    }

    int vx = 0;
    if (state < Won) {
        if (state < Diving) vx = local.x * 6;
        else vx = divingDirection * 8;
    }
    int futureX = x + vx;
    x = futureX;
    if (!isPlayer2) {
        if (futureX < PLAYER_HALF_LENGTH) x = PLAYER_HALF_LENGTH;
        else if (futureX > GROUND_HALF_WIDTH - PLAYER_HALF_LENGTH) x = GROUND_HALF_WIDTH - PLAYER_HALF_LENGTH;
    } else {
        if (futureX < GROUND_HALF_WIDTH + PLAYER_HALF_LENGTH) x = GROUND_HALF_WIDTH + PLAYER_HALF_LENGTH;
        else if (futureX > GROUND_WIDTH - PLAYER_HALF_LENGTH) x = GROUND_WIDTH - PLAYER_HALF_LENGTH;
    }

    if (state < Diving && local.y == -1 && y == PLAYER_GROUND_Y) {
        yVelocity = -16;
        state = Jumping;
        frameNumber = 0;
        m_pSoundChu->play((x - GROUND_HALF_WIDTH) * 100);
    }

    y += yVelocity;
    if (y < PLAYER_GROUND_Y) {
        yVelocity += 1;
    } else if (y > PLAYER_GROUND_Y) {
        yVelocity = 0;
        y = PLAYER_GROUND_Y;
        frameNumber = 0;
        if (state == Diving) {
            state = LyingDown;
            frameNumber = 0;
            lyingDownTimer = 3;
        } else {
            state = Normal;
        }
    }

    if (local.power == 1) {
        if (state == Jumping) {
            delayBeforeNextFrame = 5;
            frameNumber = 0;
            state = PowerHitJumping;
            m_pSoundPika->play((x - GROUND_HALF_WIDTH) * 100);
        } else if (state == Normal && local.x != 0) {
            state = Diving;
            frameNumber = 0;
            divingDirection = local.x;
            yVelocity = -5;
            m_pSoundChu->play((x - GROUND_HALF_WIDTH) * 100);
        }
    }

    if (state == Jumping) {
        frameNumber = (frameNumber + 1) % 3;
    } else if (state == PowerHitJumping) {
        if (delayBeforeNextFrame < 1) {
            frameNumber += 1;
            if (frameNumber > 4) {
                frameNumber = 0;
                state = Jumping;
            }
        } else {
            delayBeforeNextFrame -= 1;
        }
    } else if (state == Normal) {
        delayBeforeNextFrame += 1;
        if (delayBeforeNextFrame > 3) {
            delayBeforeNextFrame = 0;
            int next = frameNumber + armDirection;
            if (next < 0 || next > 4) armDirection = -armDirection;
            frameNumber += armDirection;
        }
    }

    if (gameEnded) {
        if (state == Normal) {
            if (isWinner) {
                state = Won;
                m_pSoundPipi->play((x - GROUND_HALF_WIDTH) * 100);
            } else {
                state = Lost;
            }
            delayBeforeNextFrame = 0;
            frameNumber = 0;
        }
        updateWinLoseFrame();
    }
}

void CPlayer::updateWinLoseFrame()
{
    if (gameEnded && frameNumber < 4) {
        delayBeforeNextFrame += 1;
        if (delayBeforeNextFrame > 4) {
            delayBeforeNextFrame = 0;
            frameNumber += 1;
        }
    }
}

void CPlayer::runAI(int otherPlayerX, const int* ballState, CUserInput& input)
{
    input = CUserInput();
    int ballX = ballState[0], ballY = ballState[1];
    int ballVX = ballState[2], ballVY = ballState[3], expectedX = ballState[4];

    int targetX = expectedX;
    if (iabs(ballX - x) > 100 && iabs(ballVX) < 5) {
        int left = isPlayer2 * GROUND_HALF_WIDTH;
        if ((targetX <= left || targetX >= isPlayer2 * GROUND_WIDTH + GROUND_HALF_WIDTH)
            && computerWhereToStandBy == 0) {
            targetX = left + GROUND_HALF_WIDTH / 2;
        }
    }

    if (iabs(targetX - x) > computerBoldness + 8) {
        input.x = isign(targetX - x);
    } else if (rand_range(20) == 0) {
        computerWhereToStandBy = rand_range(2);
    }

    if (state == Normal) {
        if (iabs(ballVX) < computerBoldness + 3 &&
            iabs(ballX - x) < PLAYER_HALF_LENGTH &&
            -36 < ballY && ballY < computerBoldness * 10 + 84 &&
            ballVY > 0) {
            input.y = -1;
        }

        int left = isPlayer2 * GROUND_HALF_WIDTH;
        int right = (isPlayer2 * 3 + 3) * 0x48;   // 216 or 432
        if (left < expectedX && expectedX < right &&
            iabs(ballX - x) > computerBoldness * 5 + PLAYER_LENGTH &&
            left < ballX && ballX < right &&
            ballY > 174) {
            input.power = 1;
            input.x = isign(ballX - x);
        }
    } else if (state == Jumping || state == PowerHitJumping) {
        if (iabs(ballX - x) > 8) input.x = isign(ballX - x);
        if (iabs(ballX - x) < 48 && iabs(ballY - y) < 48) {
            if (decidePowerHit(otherPlayerX, ballState, input)) {
                input.power = 1;
                if (iabs(otherPlayerX - x) < 80 && input.y != -1) input.y = -1;
            }
        }
    }
}

bool CPlayer::decidePowerHit(int otherPlayerX, const int* ballState, CUserInput& input)
{
    int xDirections[3] = { 1, 0, -1 };
    int yDirections[3];
    if (rand_range(2) == 0) {
        yDirections[0] = -1; yDirections[1] = 0; yDirections[2] = 1;
    } else {
        yDirections[0] = 1; yDirections[1] = 0; yDirections[2] = -1;
    }
    int left = isPlayer2 * GROUND_HALF_WIDTH;
    int right = isPlayer2 * GROUND_WIDTH + GROUND_HALF_WIDTH;
    for (int xd : xDirections) {
        for (int yd : yDirections) {
            int landing = expectedWhenPowerHit(xd, yd, ballState);
            if ((landing <= left || landing >= right) &&
                iabs(landing - otherPlayerX) > PLAYER_LENGTH) {
                input.x = xd;
                input.y = yd;
                return true;
            }
        }
    }
    return false;
}

int CPlayer::expectedWhenPowerHit(int xDirection, int yDirection, const int* ballState) const
{
    int cx = ballState[0], cy = ballState[1];
    int cvx = ballState[2], cvy = ballState[3];
    if (cx < GROUND_HALF_WIDTH) cvx = (iabs(xDirection) + 1) * 10;
    else cvx = -(iabs(xDirection) + 1) * 10;
    cvy = iabs(cvy) * yDirection * 2;

    for (int guard = 0; guard < 1000; guard++) {
        if (cx + cvx < BALL_RADIUS || cx + cvx > GROUND_WIDTH) cvx = -cvx;
        if (cy + cvy < 0) cvy = 1;
        if (iabs(cx - GROUND_HALF_WIDTH) < NET_HALF_WIDTH && cy > NET_TOP_TOP_Y) {
            // Only the vertical branch is handled: the prediction cannot
            // account for a horizontal net bounce.
            if (cy < NET_TOP_BOTTOM_Y && cvy > 0) cvy = -cvy;
        }
        cy += cvy;
        if (cy > BALL_GROUND_Y) return cx;
        cx += cvx;
        cvy += 1;
    }
    return cx;
}

void CPlayer::draw(CScreen* screen)
{
    int index = frameNumber + state * 5;
    if (state == Diving) index = 15;
    else if (state == LyingDown) index = 20;
    if (index < 0 || index >= 35 || !m_pSprite[index]) return;

    int flags;
    if (state == Diving || state == LyingDown) {
        flags = (divingDirection >= 0) ? 0 : 1;
    } else {
        flags = isPlayer2;
    }
    m_pSprite[index]->draw(screen, x - 32, y - 32, flags);
}

void CPlayer::getRect(RECT* out) const
{
    out->left = x - PLAYER_HALF_LENGTH;
    out->top = y - PLAYER_HALF_LENGTH;
    out->right = PLAYER_LENGTH;
    out->bottom = PLAYER_LENGTH;
}

void CPlayer::clearQueue() { m_pQueue->flush(nullptr); }
