#include "court.h"
#include "resource_ids.h"

CCourt::CCourt()
{
    m_pSheet = new CSpriteSheet();
    m_pCloudSprite = new CSprite();
    m_pWaveSprite = new CSprite();
    m_pDigits = new CSpriteSheet();
    for (auto& s : m_pDigit) s = new CSprite();
    m_pQueue = new CRenderQueue();
}

CCourt::~CCourt()
{
    delete m_pSheet;
    delete m_pCloudSprite;
    delete m_pWaveSprite;
    delete m_pDigits;
    for (auto& s : m_pDigit) delete s;
    delete m_pQueue;
}

bool CCourt::init(CScreen* screen1, CScreen* screen2)
{
    m_pScreen1 = screen1;
    m_pScreen2 = screen2;
    if (!m_pSheet->load(BMP_COURT)) return false;

    RECT r = { 0, 0x10, 0x30, 0x18 };
    m_pCloudSprite->setRect(m_pSheet, r.left, r.top, r.right, r.bottom);
    r = { 0x68, 0, 0x10, 0x20 };
    m_pWaveSprite->setRect(m_pSheet, r.left, r.top, r.right, r.bottom);

    if (!m_pDigits->load(BMP_DIGITS)) return false;
    for (int i = 0; i < 10; i++) {
        m_pDigit[i]->setRect(m_pDigits, i * 32, 0, 32, 32);
    }

    for (int i = 0; i < 10; i++) {
        m_cloudX[i] = rand_range(GROUND_WIDTH);
        m_cloudY[i] = rand_range(0x98);
        m_cloudSpeed[i] = rand_range(2) + 1;
        m_cloudFrame[i] = rand_range(11);
    }
    m_waveOffset = 0;
    m_waveDirection = 2;
    return true;
}

void CCourt::drawBackground(CScreen* screen)
{
    // 1) fill with the 16x16 sky tile at (16,0)
    for (int y = 0; y < GROUND_HEIGHT; y += 16)
        for (int x = 0; x < GROUND_WIDTH; x += 16)
            m_pSheet->drawRect(screen, 0x10, 0, 16, 16, x, y);

    // 2) full-width landscape strip: 72 lines starting at source y=48.  The
    //    bitmap is 112 lines tall, so the last 8 lines are clipped.
    m_pSheet->drawRect(screen, 0, 0x30, 0x1B0, 0x48, 0, 0xBC);

    // 3) ground tiles
    for (int y = 0x138; y > 0xE8; y -= 0x10) {
        int sx = (y == 0xF8) ? 0x58 : 0;
        for (int x = 0; x < GROUND_WIDTH; x += 16)
            m_pSheet->drawRect(screen, sx, 0, 16, 16, x, y);
    }

    // 4) bottom row
    for (int i = 0; i < 0x1B0 / 16; i++) {
        int sx = ((i > 0 ? 1 : 0) + (i > 25 ? 1 : 0)) * 16 + 0x28;
        m_pSheet->drawRect(screen, sx, 0, 16, 16, i * 16, 0x108);
    }

    // 5) net pole: 13 8x8 sprites at x=0xD5.  12 pole segments use the
    //    tile at (0x20,8), the top tile (0x20,0) is the red/white cap.
    for (int i = 0; i < 13; i++) {
        int sy = (i == 12) ? 0 : 8;
        CSprite tile;
        tile.setRect(m_pSheet, 0x20, sy, 8, 8);
        tile.draw(screen, 0xD5, (0x22 - i) * 8, 0);
    }
}

void CCourt::updateAnimation(CScreen* screen)
{
    static const int frameSize[11] = { 0, 1, 2, 3, 4, 4, 3, 2, 1, 0, 5 };

    for (int i = 0; i < 10; i++) {
        m_cloudX[i] += m_cloudSpeed[i];
        if (m_cloudX[i] > GROUND_WIDTH) {
            m_cloudX[i] = -0x44;
            m_cloudY[i] = rand_range(0x98);
            m_cloudSpeed[i] = rand_range(2) + 1;
        }
        m_cloudFrame[i] = (m_cloudFrame[i] + 1) % 11;
        int sz = frameSize[m_cloudFrame[i]];
        m_pCloudSprite->drawScaled(screen,
            m_cloudX[i] - sz, m_cloudY[i] - sz,
            sz * 2 + 0x30, sz * 2 + 0x18, 0);
    }

    m_waveOffset += m_waveDirection;
    if (m_waveOffset > 0x20) {
        m_waveOffset = 0x20;
        m_waveDirection = -1;
    } else if (m_waveOffset < 0 && m_waveDirection < 0) {
        m_waveDirection = 2;
        m_waveOffset = -rand_range(0x28);
    }
    for (int x = 0; x < GROUND_WIDTH; x += 16) {
        int y = (rand_range(3) - m_waveOffset) + 0x13A;
        m_pWaveSprite->draw(screen, x, y, 0);
    }
}

static void drawNumber(CCourt& court, CScreen* screen, int x, int y, int value)
{
    if (value / 10 != 0) {
        court.m_pDigit[(value / 10) % 10]->draw(screen, x, y, 0);
    }
    court.m_pDigit[value % 10]->draw(screen, x + 32, y, 0);
}

void CCourt::drawScore(CScreen* screen, int score1, int score2)
{
    drawNumber(*this, screen, 0x0E, 10, score1);
    drawNumber(*this, screen, 0x162, 10, score2);
}
