#include "game.h"
#include "resource_ids.h"

extern HINSTANCE g_hInstance;

CGame::CGame()
{
    m_pScreen1 = new CScreen();
    m_pScreen2 = new CScreen();
    m_pMusic = new CMciMusic();
    m_pMatch = new CMatchScene();
    m_pTitle = new CTitleScene();
}

CGame::~CGame()
{
    delete m_pScreen1;
    delete m_pScreen2;
    delete m_pMusic;
    delete m_pMatch;
    delete m_pTitle;
}

bool CGame::init(HWND hwnd)
{
    m_hwnd = hwnd;
    HDC hdc = GetDC(hwnd);

    // The palette comes from the 1x1 bitmap resource 0x85.
    CSpriteSheet paletteSource;
    if (!paletteSource.load(BMP_BLACK_PIXEL)) {
        ReleaseDC(hwnd, hdc);
        return false;
    }
    if (!m_pScreen1->create(hdc, SCREEN_WIDTH, SCREEN_HEIGHT, paletteSource.palette())) {
        ReleaseDC(hwnd, hdc);
        return false;
    }
    if (!m_pScreen2->create(hdc, SCREEN_WIDTH, SCREEN_HEIGHT, paletteSource.palette())) {
        ReleaseDC(hwnd, hdc);
        return false;
    }
    ReleaseDC(hwnd, hdc);

    if (!m_pMatch->init(m_pScreen1, m_pScreen2)) return false;
    if (!m_pTitle->init(m_pScreen1, m_pScreen2)) return false;

    reset();
    return true;
}

void CGame::reset()
{
    stopBgm();
    m_scene = 0;
    m_pTitle->reset();
    m_maxScore = 15;
}

void CGame::update(const CUserInput* inputs)
{
    if (m_scene == 0) {
        int result = m_pTitle->update(inputs);
        if (result != 0) {
            m_scene = 1;
            bool p1AI = true, p2AI = true;
            if (result == 8) { p1AI = true; p2AI = true; }
            else if (result & 4) { p1AI = false; p2AI = false; }
            else if (result & 1) { p1AI = false; p2AI = true; }
            else { p1AI = true; p2AI = false; }
            m_pMatch->startMatch(p1AI, p2AI, m_maxScore);
            playBgm("bgm02.mid");
        }
    } else {
        if (m_pMatch->update(inputs) == 1) reset();
    }
}

bool CGame::playBgm(const char* filename)
{
    if (!m_pMusic->open(filename)) return false;
    return m_pMusic->play();
}

void CGame::stopBgm()
{
    m_pMusic->stop();
    m_pMusic->close();
}