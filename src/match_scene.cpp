#include "match_scene.h"
#include "resource_ids.h"

CMatchScene::CMatchScene()
{
    m_pCourt = new CCourt();
    m_pPlayer[0] = new CPlayer();
    m_pPlayer[1] = new CPlayer();
    m_pBall = new CBall();
    m_pShadowSheet = new CSpriteSheet();
    m_pReadySheet = new CSpriteSheet();
    m_pStartSheet = new CSpriteSheet();
    m_pGameSetSheet = new CSpriteSheet();
    m_pShadow = new CSprite();
    m_pReadySprite = new CSprite();
    m_pStartSprite = new CSprite();
    m_pGameSetSprite = new CSprite();
    m_pQueue = new CRenderQueue();
}

CMatchScene::~CMatchScene()
{
    delete m_pCourt;
    delete m_pPlayer[0];
    delete m_pPlayer[1];
    delete m_pBall;
    delete m_pShadowSheet;
    delete m_pReadySheet;
    delete m_pStartSheet;
    delete m_pGameSetSheet;
    delete m_pShadow;
    delete m_pReadySprite;
    delete m_pStartSprite;
    delete m_pGameSetSprite;
    delete m_pQueue;
}

bool CMatchScene::init(CScreen* screen1, CScreen* screen2)
{
    m_pScreen1 = screen1;
    m_pScreen2 = screen2;
    if (!m_pPlayer[0]->init(screen1, screen2, 0)) return false;
    if (!m_pPlayer[1]->init(screen1, screen2, 1)) return false;
    if (!m_pBall->init(screen1, screen2)) return false;
    if (!m_pCourt->init(screen1, screen2)) return false;

    if (!m_pShadowSheet->load(BMP_SHADOW)) return false;
    m_pShadow->setRect(m_pShadowSheet, 0, 0, m_pShadowSheet->width(), m_pShadowSheet->height());

    m_pStartSheet->load(BMP_GAME_START);
    m_pStartSprite->setRect(m_pStartSheet, 0, 0, m_pStartSheet->width(), m_pStartSheet->height());
    m_pReadySheet->load(BMP_READY);
    m_pReadySprite->setRect(m_pReadySheet, 0, 0, m_pReadySheet->width(), m_pReadySheet->height());
    m_pGameSetSheet->load(BMP_GAME_SET);
    m_pGameSetSprite->setRect(m_pGameSetSheet, 0, 0, m_pGameSetSheet->width(), m_pGameSetSheet->height());
    m_pQueue->reset();
    return true;
}

bool CMatchScene::startMatch(bool player1Computer, bool player2Computer, int maxScore)
{
    m_timer = 0;
    m_state = StateGameStart;
    m_maxScore = maxScore;
    m_score[0] = m_score[1] = 0;
    m_serveSide = 0;
    m_pCourt->m_pQueue->reset();
    m_pPlayer[0]->setSide(0);
    m_pPlayer[1]->setSide(1);
    // Player 1 plays on the left, player 2 on the right.
    m_pPlayer[0]->isComputer = player1Computer;
    m_pPlayer[1]->isComputer = player2Computer;
    resetRound(0);
    return true;
}

void CMatchScene::resetRound(int serveSide)
{
    m_serveSide = serveSide;
    m_pPlayer[0]->initRound();
    m_pPlayer[1]->initRound();
    m_pBall->initForNewRound(serveSide != 0);
}

bool CMatchScene::physicsStep(const CUserInput* inputs)
{
    bool grounded = m_pBall->updateWorld();

    // inputs[0] is player 1 (arrows/Enter), inputs[1] is player 2 (RDFG/Z).
    // Player 1 owns the left-hand character, player 2 the right-hand one.
    int ballState[5];
    for (int i = 0; i < 2; i++) {
        m_pBall->getState(ballState);           // copy + predict landing
        int other = m_pPlayer[1 - i]->x;
        m_pPlayer[i]->update(inputs[i], other, ballState);
    }

    for (int i = 0; i < 2; i++) {
        CPlayer* p = m_pPlayer[i];
        if (m_pBall->hitTest(p->x, p->y)) {
            if (!p->ballHitLatch) {
                m_pBall->hitByPlayer(p->x, inputs[i], p->state);
                p->ballHitLatch = 1;
            }
        } else {
            p->ballHitLatch = 0;
        }
    }
    return grounded;
}

void CMatchScene::drawGameStartMessage(CScreen* screen)
{
    // Grow from the center to 2x size and stay there.
    //   iVar1 = timer * width  / 50
    //   iVar2 = timer * height / 50
    //   rect  = (216 - iVar1, 50 + 2*iVar2, 2*iVar1, 2*iVar2)
    int w = m_pStartSheet->width(), h = m_pStartSheet->height();
    int iVar1 = (m_timer * w) / 50;
    int iVar2 = (m_timer * h) / 50;
    m_pStartSprite->drawScaled(screen,
                               GROUND_HALF_WIDTH - iVar1,
                               50 + iVar2 * 2,
                               iVar1 * 2, iVar2 * 2, 0);
}

void CMatchScene::drawReadyMessage(CScreen* screen)
{
    // Centered at x=216, y=50 - height/2, blinking every 10 ticks.
    if (m_timer % 10 > 4) {
        int w = m_pReadySheet->width(), h = m_pReadySheet->height();
        m_pReadySprite->draw(screen, GROUND_HALF_WIDTH - w / 2, 0x32 - h / 2, 0);
    }
}

void CMatchScene::drawGameSetMessage(CScreen* screen)
{
    // The first 50 ticks shrink a huge centered copy down to 1x.
    int w = m_pGameSetSheet->width(), h = m_pGameSetSheet->height();
    if (m_timer < 50) {
        int iVar1 = (w * (50 - m_timer)) / 50;
        int iVar2 = (h * (50 - m_timer)) / 50;
        int x = (0x6C - iVar1) * 2 - w / 2;
        int y = (0x19 - iVar2) * 2;
        m_pGameSetSprite->drawScaled(screen, x, y,
                                     iVar1 * 4 + w, iVar2 * 4 + h, 0);
    } else {
        m_pGameSetSprite->draw(screen, GROUND_HALF_WIDTH - w / 2, 0x32, 0);
    }
}

int CMatchScene::update(const CUserInput* inputs)
{
    if (m_state == StateSlowMotion) {
        // Slow motion holds for 200 ms on the first six ticks.
        if (m_timer < 6) Sleep(200);
        if (m_timer > 10) {
            m_state = StateReady;
            m_timer = 0;
            resetRound(m_serveSide);
        }
        m_timer++;
    } else if (m_state == StateGameStart) {
        m_timer++;
        if (m_timer > 70) { m_state = StateRally; m_timer = 0; }
    } else if (m_state == StateReady) {
        drawReadyMessage(m_pScreen1);
        m_timer++;
        if (m_timer > 30) { m_state = StateRally; m_timer = 0; }
    } else if (m_state == StateQuit) {
        return 1;
    }

    if (m_state == StateRally || m_state == StateSlowMotion || m_state == StateGameSet) {
        bool grounded = physicsStep(inputs);

        if (m_state == StateGameSet) {
            drawGameSetMessage(m_pScreen1);
            if (m_timer > 70 && (inputs[0].power == 1 || inputs[1].power == 1)) {
                m_state = StateQuit;
            }
            m_timer++;
            if (m_timer > 210) m_state = StateQuit;
        }

        if (grounded && m_state == StateRally) {
            int scorer = (m_pBall->x < GROUND_HALF_WIDTH + 1) ? 1 : 0;
            m_score[scorer]++;
            m_serveSide = scorer;
            if (m_score[0] >= m_maxScore || m_score[1] >= m_maxScore) {
                m_state = StateGameSet;
                m_timer = 0;
                bool p1Won = (m_score[0] >= m_maxScore);
                m_pPlayer[0]->setGameEnd(p1Won);
                m_pPlayer[1]->setGameEnd(!p1Won);
            } else {
                m_state = StateSlowMotion;
                m_timer = 0;
            }
        }
    }

    // --- draw frame ---------------------------------------------------------
    CScreen* screen = m_pScreen1;
    m_pCourt->drawBackground(screen);
    m_pCourt->updateAnimation(screen);

    m_pShadow->draw(screen, m_pPlayer[0]->x - 16, 269, 0);
    m_pShadow->draw(screen, m_pPlayer[1]->x - 16, 269, 0);
    m_pShadow->draw(screen, m_pBall->x - 12, 269, 0);
    m_pPlayer[0]->draw(screen);
    m_pPlayer[1]->draw(screen);
    m_pBall->draw(screen);
    m_pCourt->drawScore(screen, m_score[0], m_score[1]);


    if (m_state == StateGameStart) drawGameStartMessage(screen);
    else if (m_state == StateReady) drawReadyMessage(screen);
    else if (m_state == StateGameSet) drawGameSetMessage(screen);
    return 0;
}