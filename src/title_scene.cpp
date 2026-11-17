#include "title_scene.h"
#include "resource_ids.h"

CTitleScene::CTitleScene()
{
    m_pLogoSheet = new CSpriteSheet();
    m_pTextSheet = new CSpriteSheet();
    m_pMegaSheet = new CSpriteSheet();
    m_pControlSheet = new CSpriteSheet();
    m_pFaceSheet = new CSpriteSheet();
    m_pArtSheet = new CSpriteSheet();
    m_pCopyrightSheet = new CSpriteSheet();
    m_pLogoSprite = new CSprite();
    m_pTextSprite = new CSprite();
    m_pMegaSprite = new CSprite();
    m_pControlSprite[0] = new CSprite();
    m_pControlSprite[1] = new CSprite();
    m_pArtSprite = new CSprite();
    m_pCopyrightSprite = new CSprite();
    m_pSound1 = new CSound();
    m_pSound2 = new CSound();
}

CTitleScene::~CTitleScene()
{
    delete m_pLogoSheet; delete m_pLogoSprite;
    delete m_pTextSheet; delete m_pTextSprite;
    delete m_pMegaSheet; delete m_pMegaSprite;
    delete m_pControlSheet;
    delete m_pControlSprite[0]; delete m_pControlSprite[1];
    delete m_pFaceSheet; delete m_pArtSheet; delete m_pArtSprite;
    delete m_pCopyrightSheet; delete m_pCopyrightSprite;
    delete m_pSound1; delete m_pSound2;
}

bool CTitleScene::init(CScreen* screen1, CScreen* screen2)
{
    m_pScreen1 = screen1;
    m_pScreen2 = screen2;

    if (!m_pLogoSheet->load(BMP_TITLE_LOGO)) return false;
    m_pLogoSprite->setRect(m_pLogoSheet, 0, 0,
                           m_pLogoSheet->width(), m_pLogoSheet->height());

    if (!m_pTextSheet->load(BMP_POKEMON_LOGO)) return false;
    m_pTextSprite->setRect(m_pTextSheet, 0, 0,
                           m_pTextSheet->width(), m_pTextSheet->height());

    if (!m_pMegaSheet->load(BMP_MEGA)) return false;
    m_pMegaSprite->setRect(m_pMegaSheet, 0, 0,
                           m_pMegaSheet->width(), m_pMegaSheet->height());

    if (!m_pControlSheet->load(BMP_CONTROLS)) return false;
    // 0x7D: two 120x20 text lines at y=0 and y=20.
    m_pControlSprite[0]->setRect(m_pControlSheet, 0, 0, 120, 20);
    m_pControlSprite[1]->setRect(m_pControlSheet, 0, 20, 120, 20);

    if (!m_pFaceSheet->load(BMP_PIKA_FACE)) return false;
    if (!m_pArtSheet->load(BMP_TITLE_ART)) return false;
    m_pArtSprite->setRect(m_pArtSheet, 0, 0,
                          m_pArtSheet->width(), m_pArtSheet->height());

    if (!m_pCopyrightSheet->load(BMP_COPYRIGHT)) return false;
    m_pCopyrightSprite->setRect(m_pCopyrightSheet, 0, 0,
                                m_pCopyrightSheet->width(), m_pCopyrightSheet->height());

    m_pSound1->load(WAV_TITLE_1);
    m_pSound2->load(WAV_TITLE_2);
    reset();
    return true;
}

void CTitleScene::reset()
{
    m_phase = 0;
    m_menuPhase = 0;
    m_bigTimer = 0;
    m_logoTimer = 0;
    m_scroll = 0;
    m_introTimer = 0;
    m_selection = 0;
    m_animTimer = 0;
    m_result = 0;
    m_idleTimer = 0;
    m_powerLatch = true;   // ignore a power key that is already held

    m_pScreen1->clear(0);
    // The intro screen is the 88x110 "man with a briefcase" art at (172,97).
    m_pArtSprite->draw(m_pScreen1, 0xAC, 0x61, 0);
}

int CTitleScene::update(const CUserInput* inputs)
{
    // Rising-edge detection.  The press that leaves the intro must not be
    // seen by the title menu on the next frame; otherwise the player goes
    // intro -> title -> match with a single held power key.
    bool power = (inputs[0].power == 1) || (inputs[1].power == 1);
    bool powerPressed = power && !m_powerLatch;
    m_powerLatch = power;

    if (m_phase == 0) {
        if (powerPressed || m_introTimer > 0xA4) {
            m_phase = 1;
            m_menuPhase = 0;
            m_bigTimer = 0;
            m_logoTimer = 0;
            m_animTimer = 0;
            m_selection = 0;
            m_idleTimer = 0;
            m_pScreen1->clear(0);
        } else {
            m_introTimer++;
        }
        // Always return: never process the same press again as a title input.
        return 0;
    }

    // --- menu/selection -----------------------------------------------------
    if (m_phase == 1) {
        if (powerPressed) {
            if (m_menuPhase <= 3) {
                // Skip the logo animation and show the menu, but consume this
                // press.  A second, separate press starts the match.
                m_menuPhase = 4;
                m_animTimer = 10;
            } else {
                m_result = (inputs[0].power == 1) ? 1 : 2;
                if (m_selection == 1) m_result |= 4;
                if (inputs[0].x != 0 || inputs[1].x != 0 ||
                    inputs[0].y != 0 || inputs[1].y != 0) m_result = 8;
                m_menuPhase = 5;
                m_pSound1->play(0);
                m_phase = 2;
                return m_result;
            }
        }

        if (m_menuPhase > 3) {
            int old = m_selection;
            if (inputs[0].x > 0 || inputs[1].x > 0 ||
                inputs[0].y > 0 || inputs[1].y > 0) m_selection = 1;
            else if (inputs[0].x < 0 || inputs[1].x < 0 ||
                     inputs[0].y < 0 || inputs[1].y < 0) m_selection = 0;
            if (old != m_selection) {
                m_animTimer = 2;
                m_pSound2->play(0);  // : selection move
            }
            if (m_animTimer < 10) m_animTimer++;
        }

        m_idleTimer++;
        if (m_idleTimer > 300) {
            reset();
            return 0;
        }

        // --- draw title -----------------------------------------------------
        m_pScreen1->clear(0);
        drawScrollingBackground(m_pScreen1);
        drawBigLogo(m_pScreen1);
        drawLogo(m_pScreen1);
        drawMenu(m_pScreen1);

        int w = m_pCopyrightSheet->width();
        m_pCopyrightSprite->draw(m_pScreen1, GROUND_HALF_WIDTH - w / 2, 0x108, 0);
        return 0;
    }
    return 0;
}

void CTitleScene::drawScrollingBackground(CScreen* screen)
{
    int w = m_pFaceSheet->width();
    int h = m_pFaceSheet->height();
    m_scroll = (m_scroll + 2) % h;

    int cols = GROUND_WIDTH / w + 2;
    int rows = GROUND_HEIGHT / h + 3;
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            m_pFaceSheet->drawRect(screen, 0, 0, w, h,
                                   col * w - m_scroll,
                                   row * h - m_scroll);
        }
    }
}

void CTitleScene::drawLogo(CScreen* screen)
{
    int logoW = m_pLogoSheet->width();
    int logoH = m_pLogoSheet->height();

    if (m_menuPhase == 1) {
        m_logoTimer -= 0x0F;
        m_pLogoSprite->draw(screen, 0x50, m_logoTimer + 0x7D, 0);
        if (m_logoTimer < 0) {
            m_menuPhase = 2;
            m_logoTimer = 200;
        }
    } else if (m_menuPhase == 2) {
        m_logoTimer -= 0x0F;
        int w = m_logoTimer;
        if (w < 0) w = 0;
        m_pLogoSprite->drawScaled(screen, 0x8C, 0x50, w, logoH, 0);
        if (m_logoTimer < 0x32) {
            m_logoTimer = 0x28;
            m_menuPhase = 3;
        }
    } else if (m_menuPhase == 3) {
        m_logoTimer += 0x0F;
        int w = m_logoTimer;
        if (w > logoW) w = logoW;
        m_pLogoSprite->drawScaled(screen, 0x8C, 0x50, w, logoH, 0);
        if (logoW < m_logoTimer) m_menuPhase = 4;
    } else if (m_menuPhase > 3) {
        m_pLogoSprite->draw(screen, 0x8C, 0x50, 0);
        m_pTextSprite->draw(screen, 0xAA, 0x28, 0);
    }
}

void CTitleScene::drawBigLogo(CScreen* screen)
{
    static const int pulse[10] = { 20, 20, 22, 25, 27, 30, 27, 25, 22, 20 };
    int w = m_pMegaSheet->width();
    int h = m_pMegaSheet->height();

    if (m_menuPhase == 0) {
        m_bigTimer++;
        int halfW = (m_bigTimer * w) / 0x3C;
        int halfH = (m_bigTimer * h) / 0x3C;
        m_pMegaSprite->drawScaled(screen, 100 - halfW, 0x46 - halfH,
                                  halfW * 2, halfH * 2, 0);
        if (m_bigTimer > 0x1D) {
            m_menuPhase = 1;
            m_logoTimer = 200;
        }
    } else {
        m_bigTimer = (m_bigTimer + 1) % 9;
        int halfW = (w * pulse[m_bigTimer]) / 0x3C;
        int halfH = (h * pulse[m_bigTimer]) / 0x3C;
        m_pMegaSprite->drawScaled(screen, 100 - halfW, 0x46 - halfH,
                                  halfW * 2, halfH * 2, 0);
    }
}

void CTitleScene::drawMenu(CScreen* screen)
{
    if (m_menuPhase <= 3) return;

    int sheetW = m_pControlSheet->width();
    int baseX = GROUND_HALF_WIDTH - sheetW / 2;
    int y = 0xB8;   // 184

    for (int i = 0; i < 2; i++) {
        int active = (m_selection == i) ? 1 : 0;
        int anim = active ? m_animTimer : 0;
        int growX = (anim + 2) * active;
        int growY = anim * active;
        int dx = baseX - growX;
        int dy = y - growY;
        int dw = sheetW + growX * 2;
        int dh = 20 + growY * 2;
        m_pControlSprite[i]->drawScaled(screen, dx, dy, dw, dh, 0);
        y += 0x1E;   // 30
    }
}
