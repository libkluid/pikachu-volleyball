#include "input.h"

void CKeyboard::clear()
{
    memset(m_state, 0, sizeof(m_state));
    memset(m_keymap, 0, sizeof(m_keymap));
}

void CKeyboard::setKeys(int row, const BYTE* keys)
{
    if (row < 0 || row >= 8) return;
    memcpy(m_keymap + row * 32, keys, 32);
}

unsigned int CKeyboard::readRow(int row)
{
    GetKeyboardState(m_state);
    unsigned int result = 0;
    for (int i = 0; i < 32; i++) {
        BYTE vk = m_keymap[row * 32 + i];
        if (m_state[vk] & 0x80) result |= 1u << i;
    }
    return result;
}

CJoystick::CJoystick(UINT id) : m_id(id)
{
    memset(&m_caps, 0, sizeof(m_caps));
    memset(&m_info, 0, sizeof(m_info));
    joyGetDevCapsA(m_id, &m_caps, sizeof(m_caps));
    m_info.dwSize = sizeof(m_info);
    m_info.dwFlags = JOY_RETURNALL;
    if (joyGetPosEx(m_id, &m_info) == JOYERR_NOERROR) {
        m_connected = true;
        m_centerX = (m_caps.wXmin + m_caps.wXmax) / 2;
        m_centerY = (m_caps.wYmin + m_caps.wYmax) / 2;
    }
    m_scale = 1000;
}

bool CJoystick::poll()
{
    m_info.dwSize = sizeof(m_info);
    m_info.dwFlags = JOY_RETURNALL;
    return joyGetPosEx(m_id, &m_info) == JOYERR_NOERROR;
}

unsigned int CJoystick::axisMask()
{
    unsigned int mask = 0;
    int dx = (int)m_info.dwXpos - m_centerX;
    int dy = (int)m_info.dwYpos - m_centerY;
    if (dx / m_scale > 0) mask |= 0x08;
    else if (dx / m_scale < 0) mask |= 0x04;
    if (dy / m_scale > 0) mask |= 0x02;
    else if (dy / m_scale < 0) mask |= 0x01;
    mask |= (m_info.dwButtons << 4);
    return mask;
}

CPlayerInput::CPlayerInput(UINT playerId)
{
    playerIndex = (int)playerId;
    keyboard = new CKeyboard();
    joystick = new CJoystick(playerId);
    connected = joystick->m_connected;
    if (connected) joystick->setScale(5000);
    setDefaultKeys();
    repeat = 0xFFFFFFFF;
}

void CPlayerInput::setDefaultKeys()
{
    keyboard->clear();
    BYTE zero[32] = { 0 };

    if (playerIndex == 0) {
        // Player 1: arrows, Space/Enter for the power hit.
        BYTE row0[32] = {
            0x26, 0x28, 0x25, 0x27,   // up, down, left, right
            0x20, 0x0D, 0x2E, 0x1B,   // Space, Enter, Delete, Esc
        };
        keyboard->setKeys(0, row0);
        for (int i = 1; i < 8; i++) keyboard->setKeys(i, zero);
    } else {
        // Player 2: R/F/D/G + Z, plus the numeric keypad.
        BYTE row0[32] = {
            0x52, 0x46, 0x44, 0x47,   // R, F, D, G
            0x5A, 0x58,               // Z, X
        };
        BYTE row1[32] = {
            0x68, 0x62, 0x64, 0x66,   // NumPad 8, 2, 4, 6
            0x5A, 0x58,               // Z, X
        };
        keyboard->setKeys(0, row0);
        keyboard->setKeys(1, row1);
        for (int i = 2; i < 8; i++) keyboard->setKeys(i, zero);
    }
}

unsigned int CPlayerInput::poll()
{
    unsigned int mask = 0;
    if (connected) {
        joystick->poll();
        mask = joystick->axisMask();
    }
    for (int row = 0; row < 8; row++) {
        mask |= keyboard->readRow(row);
    }
    unsigned int result = (mask & ~last) | (mask & repeat);
    last = mask;
    return result;
}
