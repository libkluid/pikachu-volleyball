#pragma once
#include "pika.h"

// ---------------------------------------------------------------------------
//  CKeyboard - 256 key states + 256 key map
//  Key map layout: 8 rows * 32 bytes.  Only rows 0 and 1 are filled
//  with default keys; poll() ORs all 8 rows, which is why this build shares
//  the keyboard between both players.
// ---------------------------------------------------------------------------
class CKeyboard {
public:
    CKeyboard() { clear(); }
    void clear();
    void setKeys(int row, const BYTE* keys);
    unsigned int readRow(int row);

private:
    BYTE m_state[256];
    BYTE m_keymap[256];
};

// ---------------------------------------------------------------------------
//  CJoystick
// ---------------------------------------------------------------------------
class CJoystick {
public:
    explicit CJoystick(UINT id = 0);
    bool poll();
    void setScale(int scale) { m_scale = scale; }
    unsigned int axisMask();

    UINT m_id = 0;
    JOYCAPSA m_caps;
    JOYINFOEX m_info;
    int m_centerX = 0, m_centerY = 0;
    int m_scale = 1000;
    bool m_connected = false;
};

// ---------------------------------------------------------------------------
//  CPlayerInput
// ---------------------------------------------------------------------------
class CPlayerInput {
public:
    explicit CPlayerInput(UINT joystickId);
    unsigned int poll();
    void setDefaultKeys();

    CJoystick*  joystick = nullptr;
    CKeyboard*  keyboard = nullptr;
    bool        connected = false;
    unsigned int last = 0;
    unsigned int repeat = 0xFFFFFFFF;
    int         playerIndex = 0;      // 0 = left, 1 = right
};
