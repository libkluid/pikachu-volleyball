#pragma once
#include "pika.h"

// ---------------------------------------------------------------------------
//  8-bit DIB helpers: bitmap sheets, palettes and sprites.
// ---------------------------------------------------------------------------

class CScreen;

struct CBitmap8 {
    BITMAPINFO* m_pInfo = nullptr;   // BITMAPINFOHEADER + 256 RGBQUAD
    BYTE*       m_pBits = nullptr;   // bottom-up pixel data
    int         m_width = 0;
    int         m_height = 0;
    int         m_pitch = 0;
    HPALETTE    m_hPalette = nullptr;
    HBITMAP     m_hBitmap = nullptr;
    HDC         m_hMemDC = nullptr;

    ~CBitmap8() { destroy(); }
    void destroy();

    bool loadBitmapResource(int id);
    bool createScreen(HDC hdc, int w, int h, const RGBQUAD* palette);

    int width() const { return m_width; }
    int height() const { return m_height; }
    int pitch() const { return m_pitch; }
    BYTE* bits() const { return m_pBits; }
    const RGBQUAD* palette() const { return m_pInfo ? m_pInfo->bmiColors : nullptr; }
};

// ---------------------------------------------------------------------------
//  CSpriteSheet - bitmap resource wrapper.
// ---------------------------------------------------------------------------
class CSpriteSheet : public CBitmap8 {
public:
    CSpriteSheet() = default;
    bool load(int resourceId) { return loadBitmapResource(resourceId); }

    // rect = (x, y, w, h); flag bit0 = horizontal flip; no transparency
    // for the court/background sheet.
    void drawRect(CScreen* dst, int sx, int sy, int sw, int sh,
                  int dx, int dy, int flags = 0);
};

// ---------------------------------------------------------------------------
//  CScreen - one 8-bit DIB back buffer.
// ---------------------------------------------------------------------------
class CScreen : public CBitmap8 {
public:
    CScreen() = default;
    bool create(HDC hdc, int w, int h, const RGBQUAD* palette);
    void clear(BYTE index = 0);
    void present(HDC hdc, int x = 0, int y = 0, int w = SCREEN_WIDTH, int h = SCREEN_HEIGHT);
};

// ---------------------------------------------------------------------------
//  CSprite - a sub-rectangle of a sheet + transparency range.
// ---------------------------------------------------------------------------
class CSprite {
public:
    CSprite() = default;
    CSprite(CSpriteSheet* sheet, int x, int y, int w, int h)
        : m_pSheet(sheet), m_x(x), m_y(y), m_w(w), m_h(h) {}

    void setRect(CSpriteSheet* sheet, int x, int y, int w, int h);
    void setRect(const RECT& r, CSpriteSheet* sheet) {
        setRect(sheet, r.left, r.top, r.right, r.bottom);
    }

    void setTransparency(int low, int high) { m_low = low; m_high = high; }

    void draw(CScreen* dst, int dx, int dy, int flags = 0) const;

    void drawScaled(CScreen* dst, int dx, int dy, int dw, int dh, int flags = 0) const;

private:
    CSpriteSheet* m_pSheet = nullptr;
    int m_x = 0, m_y = 0, m_w = 0, m_h = 0;
    int m_low = 0, m_high = 0;   // default: only palette index 0 is transparent
};

// ---------------------------------------------------------------------------
//  CRenderQueue - dirty rectangle queue (50 entries) used to collect changed
//  regions.  present() copies the whole back buffer, so flush() only clears.
// ---------------------------------------------------------------------------
class CRenderQueue {
public:
    void reset() { m_count = 0; }
    bool push(const RECT& r);
    void flush(CScreen* screen);

private:
    int  m_count = 0;
    RECT m_rects[50];
};
