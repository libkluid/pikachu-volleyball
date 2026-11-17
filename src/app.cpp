#include "app.h"
#include "resource_ids.h"
#include <cstdio>

HINSTANCE g_hInstance = nullptr;
CApp* g_pApp = nullptr;
CDirectSoundMgr* g_pDirectSound = nullptr;

static const wchar_t kWindowTitle[] = L"피카츄 배구";

// Initial placement and window restore share the same code: the 432x304
// client is used as if it were the whole window, so the client ends up
// slightly below/right of the screen centre.
static void defaultWindowMetrics(int& x, int& y, int& w, int& h)
{
    DWORD style = WS_OVERLAPPEDWINDOW;
    RECT rc = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    AdjustWindowRectEx(&rc, style, TRUE, 0);
    w = rc.right - rc.left;
    h = rc.bottom - rc.top;
    x = GetSystemMetrics(SM_CXSCREEN) / 2 - 0xD8;   // 216
    y = GetSystemMetrics(SM_CYSCREEN) / 2 - 0x98;   // 152
}

static LRESULT CALLBACK PikaWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CApp* app = (CApp*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);
    if (msg == WM_CREATE) return 0;
    if (app) {
        switch (msg) {
        case WM_COMMAND:
            app->onCommand(LOWORD(wParam));
            return 0;
        case WM_KEYDOWN:
            app->onKeyDown((UINT)wParam);
            return 0;
        case WM_TIMER:
            if (wParam == 1) app->snapshotFps();
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            app->paint(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        }
    }
    if (msg == WM_DESTROY) { PostQuitMessage(0); return 0; }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

CApp::CApp()
{
    m_pSoundMgr = new CDirectSoundMgr();
    m_pInput[0] = new CPlayerInput(0);
    m_pInput[1] = new CPlayerInput(1);
    m_pGame = new CGame();
}

CApp::~CApp()
{
    timeEndPeriod(1);
    delete m_pSoundMgr;
    delete m_pInput[0];
    delete m_pInput[1];
    delete m_pGame;
}

bool CApp::createWindow(HINSTANCE hInstance, int nCmdShow)
{
    WNDCLASSA wc;
    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = PikaWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorA(nullptr, (LPCSTR)IDC_ARROW);
    wc.hIcon = LoadIconA(hInstance, IDI_PIKAICON);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = "Main";
    if (!RegisterClassA(&wc)) return false;

    DWORD style = WS_OVERLAPPEDWINDOW;
    int x, y, w, h;
    defaultWindowMetrics(x, y, w, h);

    HMENU menu = LoadMenuA(hInstance, IDR_MENU1);
    m_hwnd = CreateWindowExA(0, "Main",
                             "Pikachu Volleyball",
                             style, x, y, w, h,
                             nullptr, menu, hInstance, nullptr);
    if (!m_hwnd) return false;
    SetWindowLongPtrA(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);
    SetWindowTextW(m_hwnd, kWindowTitle);

    ShowWindow(m_hwnd, nCmdShow);
    UpdateWindow(m_hwnd);
    return true;
}

bool CApp::init(HINSTANCE hInstance, int nCmdShow)
{
    m_hInstance = hInstance;
    g_hInstance = hInstance;

    if (!createWindow(hInstance, nCmdShow)) return false;

    // 1 ms timer resolution: MsgWaitForMultipleObjects uses the system timer,
    // and the game's normal frame interval is 40 ms (25 fps).
    timeBeginPeriod(1);

    HDC hdc = GetDC(m_hwnd);
    m_pSoundMgr->init(m_hwnd, 1);
    g_pDirectSound = m_pSoundMgr;
    ReleaseDC(m_hwnd, hdc);

    if (!m_pGame->init(m_hwnd)) return false;

    SetTimer(m_hwnd, 1, 1000, nullptr);
    setPaused(false);
    return true;
}

void CApp::decodeInput(unsigned int mask, CUserInput& out)
{
    out = CUserInput();
    if (mask & 0x04) out.x = -1;
    else if (mask & 0x08) out.x = 1;
    if (mask & 0x01) out.y = -1;
    else if (mask & 0x02) out.y = 1;
    if (mask & 0x30) out.power = 1;   // Space/Enter (P1), Z/X (P2)
    if (mask & 0x20) out.extra = 1;
}

bool CApp::update()
{
    DWORD now = timeGetTime();
    if (now - m_lastUpdate < (DWORD)m_intervalMs) return false;
    m_lastUpdate = now;

    for (int i = 0; i < 2; i++) {
        unsigned int mask = m_pInput[i]->poll();
        decodeInput(mask, m_inputs[i]);
    }
    m_pGame->update(m_inputs);
    drawFrame();
    m_frameCounter++;
    return true;
}

void CApp::paint(HDC hdc)
{
    if (!hdc || !m_pGame || !m_pGame->m_pScreen1) return;

    // Stretch the 432x304 back buffer over the whole client rectangle,
    // so resizing the window scales the game image.
    RECT rc;
    GetClientRect(m_hwnd, &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    if (w > 0 && h > 0)
        m_pGame->m_pScreen1->present(hdc, 0, 0, w, h);

    if (m_debug) {
        char buf[64];
        wsprintfA(buf, "fps = %d", m_fps);
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255));
        TextOutA(hdc, 4, 4, buf, lstrlenA(buf));
    }
}

void CApp::drawFrame()
{
    HDC hdc = GetDC(m_hwnd);
    if (!hdc) return;
    paint(hdc);
    ReleaseDC(m_hwnd, hdc);
}

void CApp::startDemo()
{
    int score = 15;
    if (pika_contains(GetCommandLineA(), "--quick")) score = 1;
    m_pGame->m_scene = 1;
    m_pGame->m_pMatch->startMatch(true, true, score);
    m_pGame->playBgm("bgm02.mid");
}

void CApp::snapshotFps()
{
    m_fps = m_frameCounter;
    m_frameCounter = 0;
}

void CApp::setPaused(bool paused)
{
    m_paused = paused;
    // The global app pointer is NULL while paused, which stops App::update
    // from being called by the message loop.
    g_pApp = paused ? nullptr : this;

    wchar_t title[256];
    lstrcpyW(title, kWindowTitle);
    if (paused) lstrcatW(title, L" ( PAUSED! )");
    SetWindowTextW(m_hwnd, title);
    CheckMenuItem(GetMenu(m_hwnd), IDM_PAUSE, paused ? MF_CHECKED : MF_UNCHECKED);
}

bool CApp::onCommand(UINT id)
{
    HMENU menu = GetMenu(m_hwnd);
    switch (id & 0xFFFF) {
    case IDM_QUIT:
        PostQuitMessage(0);
        return true;
    case IDM_INFO:
        MessageBoxW(m_hwnd,
            L"\ud53c\uce74\uce04 \ubc30\uad6c \uc815\ubcf4\n\n"
            L"1997 (C) SACHI SOFT / SAWAYAKAN Programmers\n"
            L"1997 (C) Satoshi Takenouchi\n\n"
            L"C++ source port of the 1997 game",
            L"\ud53c\uce74\uce04 \ubc30\uad6c \uc815\ubcf4", MB_OK | MB_ICONINFORMATION);
        return true;
    case IDM_RESET:
        m_pGame->reset();
        return true;
    case IDM_PAUSE:
        setPaused(!m_paused);
        return true;
    case IDM_SPEED_SLOW: m_intervalMs = 50; break;
    case IDM_SPEED_NORM: m_intervalMs = 40; break;
    case IDM_SPEED_FAST: m_intervalMs = 33; break;
    case IDM_SCORE_5:  m_pGame->m_maxScore = 5;  break;
    case IDM_SCORE_10: m_pGame->m_maxScore = 10; break;
    case IDM_SCORE_15: m_pGame->m_maxScore = 15; break;
    case IDM_DEBUG: m_debug = !m_debug; return true;
    case IDM_RESTORE: {
        // Put the window back to its 448x363 size and default position.
        // A maximised window is left alone.
        if (!IsZoomed(m_hwnd)) {
            int x, y, w, h;
            defaultWindowMetrics(x, y, w, h);
            MoveWindow(m_hwnd, x, y, w, h, TRUE);
        }
        return true;
    }
    default: return false;
    }

    if (id == IDM_SPEED_SLOW || id == IDM_SPEED_NORM || id == IDM_SPEED_FAST) {
        CheckMenuItem(menu, m_speedMenuId, MF_UNCHECKED);
        CheckMenuItem(menu, id, MF_CHECKED);
        m_speedMenuId = id;
    } else if (id == IDM_SCORE_5 || id == IDM_SCORE_10 || id == IDM_SCORE_15) {
        CheckMenuItem(menu, m_scoreMenuId, MF_UNCHECKED);
        CheckMenuItem(menu, id, MF_CHECKED);
        m_scoreMenuId = id;
    }
    return true;
}

void CApp::onKeyDown(UINT vk)
{
    if (vk == VK_ESCAPE) setPaused(!m_paused);
}

int CApp::run()
{
    // Pump messages, then run App::update() when the next frame is due.
    // Instead of a busy loop this waits for either a window message or the
    // next frame deadline, so an idle game uses almost no CPU.
    MSG msg;
    for (;;) {
        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) return (int)msg.wParam;
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        // While paused (g_pApp == NULL) there is nothing to update; block
        // until the menu/keyboard wakes us.
        if (!g_pApp) {
            WaitMessage();
            continue;
        }

        DWORD elapsed = timeGetTime() - m_lastUpdate;
        DWORD interval = (DWORD)m_intervalMs;
        if (elapsed < interval) {
            // Wait for input or until the next frame deadline.
            MsgWaitForMultipleObjects(0, nullptr, FALSE,
                                     interval - elapsed, QS_ALLINPUT);
            continue;
        }

        g_pApp->update();
    }
}