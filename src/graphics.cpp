#include "graphics.h"

extern HINSTANCE g_hInstance;

void CBitmap8::destroy()
{
    if (m_hMemDC) { DeleteDC(m_hMemDC); m_hMemDC = nullptr; }
    if (m_hBitmap) { DeleteObject(m_hBitmap); m_hBitmap = nullptr; }
    if (m_hPalette) { DeleteObject(m_hPalette); m_hPalette = nullptr; }
    if (m_pBits && !m_hBitmap) { free(m_pBits); }
    m_pBits = nullptr;
    if (m_pInfo) { free(m_pInfo); m_pInfo = nullptr; }
}

bool CBitmap8::loadBitmapResource(int id)
{
    HRSRC hres = FindResourceA(g_hInstance, MAKEINTRESOURCEA(id), RT_BITMAP);
    if (!hres) return false;
    HGLOBAL hglob = LoadResource(g_hInstance, hres);
    if (!hglob) return false;
    BYTE* data = (BYTE*)LockResource(hglob);
    DWORD size = SizeofResource(g_hInstance, hres);
    if (!data || size < sizeof(BITMAPINFOHEADER)) return false;

    // RT_BITMAP resources contain the BITMAPINFOHEADER directly (no file header).
    // Tolerate a raw DIB or a DIB preceded by a 14-byte BITMAPFILEHEADER.
    const BITMAPINFOHEADER* bih = (const BITMAPINFOHEADER*)data;
    if (data[0] == 'B' && data[1] == 'M') {
        data += 14;
        size -= 14;
        bih = (const BITMAPINFOHEADER*)data;
    }

    if (bih->biBitCount != 8) return false;

    int colors = bih->biClrUsed ? (int)bih->biClrUsed : 256;
    size_t infoSize = sizeof(BITMAPINFOHEADER) + colors * sizeof(RGBQUAD);
    m_pInfo = (BITMAPINFO*)calloc(1, infoSize);
    if (!m_pInfo) return false;
    memcpy(m_pInfo, bih, sizeof(BITMAPINFOHEADER));
    memcpy(m_pInfo->bmiColors, data + sizeof(BITMAPINFOHEADER), colors * sizeof(RGBQUAD));

    m_width  = bih->biWidth;
    m_height = bih->biHeight;
    m_pitch  = ((m_width + 3) & ~3);
    size_t bitSize = (size_t)m_pitch * m_height;
    m_pBits = (BYTE*)malloc(bitSize);
    if (!m_pBits) return false;
    memcpy(m_pBits, data + infoSize, bitSize);

    // Palette object for SelectPalette when this sheet is presented directly.
    LOGPALETTE* logpal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + 255 * sizeof(PALETTEENTRY));
    logpal->palVersion = 0x300;
    logpal->palNumEntries = (WORD)colors;
    for (int i = 0; i < colors; i++) {
        logpal->palPalEntry[i].peRed   = m_pInfo->bmiColors[i].rgbRed;
        logpal->palPalEntry[i].peGreen = m_pInfo->bmiColors[i].rgbGreen;
        logpal->palPalEntry[i].peBlue  = m_pInfo->bmiColors[i].rgbBlue;
        logpal->palPalEntry[i].peFlags = 0;
    }
    m_hPalette = CreatePalette(logpal);
    free(logpal);
    return true;
}

bool CBitmap8::createScreen(HDC hdc, int w, int h, const RGBQUAD* palette)
{
    m_pInfo = (BITMAPINFO*)calloc(1, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
    if (!m_pInfo) return false;
    m_pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    m_pInfo->bmiHeader.biWidth = w;
    m_pInfo->bmiHeader.biHeight = h;      // bottom-up DIB
    m_pInfo->bmiHeader.biPlanes = 1;
    m_pInfo->bmiHeader.biBitCount = 8;
    m_pInfo->bmiHeader.biCompression = BI_RGB;
    m_pInfo->bmiHeader.biClrUsed = 256;
    if (palette) memcpy(m_pInfo->bmiColors, palette, 256 * sizeof(RGBQUAD));

    m_width = w; m_height = h; m_pitch = (w + 3) & ~3;
    void* bits = nullptr;
    m_hBitmap = CreateDIBSection(hdc, m_pInfo, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!m_hBitmap) return false;
    m_pBits = (BYTE*)bits;
    m_hMemDC = CreateCompatibleDC(hdc);
    SelectObject(m_hMemDC, m_hBitmap);

    LOGPALETTE* logpal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + 255 * sizeof(PALETTEENTRY));
    logpal->palVersion = 0x300; logpal->palNumEntries = 256;
    for (int i = 0; i < 256; i++) {
        logpal->palPalEntry[i].peRed   = m_pInfo->bmiColors[i].rgbRed;
        logpal->palPalEntry[i].peGreen = m_pInfo->bmiColors[i].rgbGreen;
        logpal->palPalEntry[i].peBlue  = m_pInfo->bmiColors[i].rgbBlue;
        logpal->palPalEntry[i].peFlags = 0;
    }
    m_hPalette = CreatePalette(logpal);
    free(logpal);
    return true;
}

bool CScreen::create(HDC hdc, int w, int h, const RGBQUAD* palette)
{
    return createScreen(hdc, w, h, palette);
}

void CScreen::clear(BYTE index)
{
    if (!m_pBits) return;
    memset(m_pBits, index, (size_t)m_pitch * m_height);
}

void CScreen::present(HDC hdc, int x, int y, int w, int h)
{
    if (!m_hMemDC) return;
    if (m_hPalette) {
        SelectPalette(m_hMemDC, m_hPalette, FALSE);
        SelectPalette(hdc, m_hPalette, FALSE);
        RealizePalette(hdc);
    }
    StretchBlt(hdc, x, y, w, h, m_hMemDC, 0, 0, m_width, m_height, SRCCOPY);
}

void CSpriteSheet::drawRect(CScreen* dst, int sx, int sy, int sw, int sh,
                            int dx, int dy, int flags)
{
    if (!dst || !m_pBits || !dst->bits()) return;
    int dstW = dst->width(), dstH = dst->height();
    for (int row = 0; row < sh; row++) {
        int ty = dy + row;
        if (ty < 0 || ty >= dstH) continue;
        int syy = sy + row;
        if (syy < 0 || syy >= m_height) continue;
        const BYTE* src = m_pBits + (m_height - 1 - syy) * m_pitch;
        BYTE* drow = dst->bits() + (dstH - 1 - ty) * dst->pitch();
        for (int col = 0; col < sw; col++) {
            int tx = dx + (flags & 1 ? (sw - 1 - col) : col);
            if (tx < 0 || tx >= dstW) continue;
            int sxx = sx + col;
            if (sxx < 0 || sxx >= m_width) continue;
            drow[tx] = src[sxx];
        }
    }
}

void CSprite::setRect(CSpriteSheet* sheet, int x, int y, int w, int h)
{
    m_pSheet = sheet; m_x = x; m_y = y; m_w = w; m_h = h;
}

void CSprite::draw(CScreen* dst, int dx, int dy, int flags) const
{
    if (!m_pSheet || !dst || !m_pSheet->bits() || !dst->bits()) return;
    int dstW = dst->width(), dstH = dst->height();
    int srcW = m_pSheet->width(), srcH = m_pSheet->height();
    int pitchS = m_pSheet->pitch();
    for (int row = 0; row < m_h; row++) {
        int ty = dy + row;
        if (ty < 0 || ty >= dstH) continue;
        int syy = m_y + row;
        if (syy < 0 || syy >= srcH) continue;
        const BYTE* src = m_pSheet->bits() + (srcH - 1 - syy) * pitchS;
        BYTE* drow = dst->bits() + (dstH - 1 - ty) * dst->pitch();
        for (int col = 0; col < m_w; col++) {
            int tx = dx + (flags & 1 ? (m_w - 1 - col) : col);
            if (tx < 0 || tx >= dstW) continue;
            int sxx = m_x + col;
            if (sxx < 0 || sxx >= srcW) continue;
            BYTE idx = src[sxx];
            if ((int)idx < m_low || (int)idx > m_high) {
                drow[tx] = idx;
            }
        }
    }
}

void CSprite::drawScaled(CScreen* dst, int dx, int dy, int dw, int dh, int flags) const
{
    if (!m_pSheet || !dst || dw <= 0 || dh <= 0) return;
    int dstW = dst->width(), dstH = dst->height();
    int srcW = m_pSheet->width(), srcH = m_pSheet->height();
    int pitchS = m_pSheet->pitch();
    for (int oy = 0; oy < dh; oy++) {
        int ty = dy + oy;
        if (ty < 0 || ty >= dstH) continue;
        int syy = m_y + (oy * m_h) / dh;
        if (syy < 0 || syy >= srcH) continue;
        const BYTE* src = m_pSheet->bits() + (srcH - 1 - syy) * pitchS;
        BYTE* drow = dst->bits() + (dstH - 1 - ty) * dst->pitch();
        for (int ox = 0; ox < dw; ox++) {
            int tx = dx + (flags & 1 ? (dw - 1 - ox) : ox);
            if (tx < 0 || tx >= dstW) continue;
            int sxx = m_x + (ox * m_w) / dw;
            if (sxx < 0 || sxx >= srcW) continue;
            BYTE idx = src[sxx];
            if ((int)idx < m_low || (int)idx > m_high) {
                drow[tx] = idx;
            }
        }
    }
}

bool CRenderQueue::push(const RECT& r)
{
    if (m_count >= 50) return false;
    m_rects[m_count++] = r;
    return true;
}

void CRenderQueue::flush(CScreen*)
{
    m_count = 0;
}