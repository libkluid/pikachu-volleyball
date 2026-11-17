#pragma once
#include "pika.h"
#include <mmsystem.h>
#include <digitalv.h>
#include <dsound.h>

// ---------------------------------------------------------------------------
//  DirectSound manager
// ---------------------------------------------------------------------------
class CDirectSoundMgr {
public:
    bool init(HWND hwnd, int priority);
    void release();
    LPDIRECTSOUND ds() const { return m_pDS; }
private:
    LPDIRECTSOUND m_pDS = nullptr;
};

// ---------------------------------------------------------------------------
//  Sound effect
// ---------------------------------------------------------------------------
class CSound {
public:
    bool load(int resourceId);
    void release();
    // pan: (x - 216) * 100, clamped to DirectSound's -10000..10000
    bool play(int pan = 0);
    void stop();
private:
    LPDIRECTSOUNDBUFFER m_pBuffer = nullptr;
};

// ---------------------------------------------------------------------------
//  MCI background music
//  Uses the external "bgm02.mid" via the "sequencer" device.
// ---------------------------------------------------------------------------
class CMciMusic {
public:
    bool open(const char* filename);
    bool play();
    bool stop();
    bool close();
private:
    MCIDEVICEID m_device = 0;
};
