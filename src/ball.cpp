#include "ball.h"
#include "resource_ids.h"
#include "pika.h"

CBall::CBall()
{
    m_pSheet = new CSpriteSheet();
    for (auto& s : m_pFrame) s = new CSprite();
    m_pEffect1 = new CSprite();
    m_pEffect2 = new CSprite();
    m_pPunch = new CSprite();
    m_pSoundHit = new CSound();
    m_pSoundGround = new CSound();
    m_pQueue = new CRenderQueue();
}

CBall::~CBall()
{
    delete m_pSheet;
    for (auto& s : m_pFrame) delete s;
    delete m_pEffect1;
    delete m_pEffect2;
    delete m_pPunch;
    delete m_pSoundHit;
    delete m_pSoundGround;
    delete m_pQueue;
}

bool CBall::init(CScreen* screen1, CScreen* screen2)
{
    if (!m_pSheet->load(BMP_BALL)) return false;

    for (int i = 0; i < 5; i++) {
        m_pFrame[i]->setRect(m_pSheet, i * 40, 0, 40, 40);
    }
    m_pEffect1->setRect(m_pSheet, 40, 40, 40, 40);
    m_pEffect2->setRect(m_pSheet, 80, 40, 40, 40);
    m_pPunch->setRect(m_pSheet, 0, 40, 40, 40);
    m_pSoundHit->load(WAV_BALL_HIT);
    m_pSoundGround->load(WAV_BALL_GROUND);
    m_pQueue->reset();
    (void)screen1; (void)screen2;
    return true;
}

void CBall::initForNewRound(bool player2Serves)
{
    x = player2Serves ? (GROUND_WIDTH - 56) : 56;
    y = 0;
    vx = 0;
    vy = 1;
    punchRadius = 0;
    isPowerHit = false;
}

void CBall::setState(const int* state)
{
    x = state[0]; y = state[1]; vx = state[2]; vy = state[3]; expectedX = state[4];
    punchRadius = 0;
    isPowerHit = false;
}

void CBall::getState(int* out) const
{
    out[0] = x; out[1] = y; out[2] = vx; out[3] = vy; out[4] = expectedX;
    const_cast<CBall*>(this)->predictLanding();
}

void CBall::predictLanding()
{
    int cx = x, cy = y, cvx = vx, cvy = vy;
    for (int guard = 0; guard < 1000; guard++) {
        if (cx + cvx < BALL_RADIUS || cx + cvx > GROUND_WIDTH) cvx = -cvx;
        if (cy + cvy < 0) cvy = 1;
        if (iabs(cx - GROUND_HALF_WIDTH) < NET_HALF_WIDTH && cy > NET_TOP_TOP_Y) {
            if (cy < NET_TOP_BOTTOM_Y) {
                if (cvy > 0) cvy = -cvy;
            } else {
                cvx = (cx < GROUND_HALF_WIDTH) ? -iabs(cvx) : iabs(cvx);
            }
        }
        cy += cvy;
        if (cy > BALL_GROUND_Y) break;
        cx += cvx;
        cvy += 1;
    }
    expectedX = cx;
}

bool CBall::updateWorld()
{
    previousPreviousX = previousX;
    previousPreviousY = previousY;
    previousX = x;
    previousY = y;

    fineRotation += vx / 2;
    if (fineRotation < 0) fineRotation += 50;
    else if (fineRotation > 50) fineRotation -= 50;
    rotation = fineRotation / 10;

    if (x - BALL_RADIUS + vx < 0 || x - BALL_RADIUS + vx > GROUND_WIDTH - BALL_RADIUS) {
        vx = -vx;
    }
    if (y + vy - BALL_RADIUS < -BALL_RADIUS) vy = 1;

    if (iabs(x - GROUND_HALF_WIDTH) < NET_HALF_WIDTH && y > NET_TOP_TOP_Y) {
        if (y <= NET_TOP_BOTTOM_Y) {
            if (vy > 0) vy = -vy;
        } else {
            vx = (x < GROUND_HALF_WIDTH) ? -iabs(vx) : iabs(vx);
        }
    }

    y += vy;
    if (y > BALL_GROUND_Y) {
        m_pSoundGround->play((x - GROUND_HALF_WIDTH) * 100);
        vy = -vy;
        punchX = x;
        y = BALL_GROUND_Y;
        punchRadius = BALL_RADIUS;
        punchY = BALL_GROUND_Y + BALL_RADIUS;
        return true;
    }
    x += vx;
    vy += 1;
    return false;
}

void CBall::draw(CScreen* screen)
{
    if (isPowerHit) {
        m_pEffect1->draw(screen, previousPreviousX - 20, previousPreviousY - 20, 0);
        m_pEffect2->draw(screen, previousX - 20, previousY - 20, 0);
    }
    if (rotation <= 4) {
        m_pFrame[rotation]->draw(screen, x - 20, y - 20, 0);
    } else {
        m_pEffect1->draw(screen, x - 20, y - 20, 0);  // hyper-ball glitch
    }
    if (punchRadius > 0) {
        m_pPunch->drawScaled(screen, punchX - punchRadius, punchY - punchRadius,
                             punchRadius * 2, punchRadius * 2, 0);
        punchRadius -= 2;
    }
}

void CBall::clearQueue() { m_pQueue->flush(nullptr); }

void CBall::getRect(RECT* out) const
{
    out->left = x - 20; out->top = y - 20; out->right = 40; out->bottom = 40;
}

bool CBall::hitTest(int px, int py) const
{
    return iabs(x - px) <= PLAYER_HALF_LENGTH && iabs(y - py) <= PLAYER_HALF_LENGTH;
}

void CBall::hitByPlayer(int playerX, const CUserInput& input, int playerState)
{
    if (x < playerX) vx = -(iabs(x - playerX) / 3);
    else if (x > playerX) vx = iabs(x - playerX) / 3;
    if (vx == 0) vx = rand_range(3) - 1;

    int absvy = iabs(vy);
    vy = -absvy;
    if (absvy < 15) vy = -15;

    if (playerState == 2) {
        if (x < GROUND_HALF_WIDTH) vx = (iabs(input.x) + 1) * 10;
        else vx = -(iabs(input.x) + 1) * 10;
        punchX = x; punchY = y;
        vy = iabs(vy) * input.y * 2;
        punchRadius = BALL_RADIUS;
        m_pSoundHit->play((x - GROUND_HALF_WIDTH) * 100);
        isPowerHit = true;
    } else {
        isPowerHit = false;
    }
    predictLanding();
}
