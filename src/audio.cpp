#include "audio.h"

extern HINSTANCE g_hInstance;

bool CDirectSoundMgr::init(HWND hwnd, int)
{
    release();
    if (FAILED(DirectSoundCreate(nullptr, &m_pDS, nullptr))) {
        m_pDS = nullptr;
        return false;
    }
    if (FAILED(m_pDS->SetCooperativeLevel(hwnd, DSSCL_PRIORITY))) {
        release();
        return false;
    }
    return true;
}

void CDirectSoundMgr::release()
{
    if (m_pDS) { m_pDS->Release(); m_pDS = nullptr; }
}

bool CSound::load(int resourceId)
{
    release();
    extern CDirectSoundMgr* g_pDirectSound;
    if (!g_pDirectSound || !g_pDirectSound->ds()) return false;

    HRSRC hres = FindResourceA(g_hInstance, MAKEINTRESOURCEA(resourceId), "WAVE");
    if (!hres) return false;
    HGLOBAL hglob = LoadResource(g_hInstance, hres);
    if (!hglob) return false;
    BYTE* data = (BYTE*)LockResource(hglob);
    DWORD size = SizeofResource(g_hInstance, hres);
    if (!data || size < 12) return false;

    // Minimal RIFF/WAVE parser: find "fmt " and "data".
    const WAVEFORMATEX* fmt = nullptr;
    const BYTE* pcm = nullptr;
    DWORD pcmSize = 0;
    DWORD off = 12;
    while (off + 8 <= size) {
        DWORD chunkSize = *(const DWORD*)(data + off + 4);
        const BYTE* body = data + off + 8;
        if (!memcmp(data + off, "fmt ", 4)) fmt = (const WAVEFORMATEX*)body;
        else if (!memcmp(data + off, "data", 4)) { pcm = body; pcmSize = chunkSize; }
        off += 8 + ((chunkSize + 1) & ~1u);
    }
    if (!fmt || !pcm) return false;

    DSBUFFERDESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME |
                   DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC;
    desc.dwBufferBytes = pcmSize;
    desc.lpwfxFormat = const_cast<WAVEFORMATEX*>(fmt);
    if (FAILED(g_pDirectSound->ds()->CreateSoundBuffer(&desc, &m_pBuffer, nullptr))) {
        m_pBuffer = nullptr;
        return false;
    }
    void* p1 = nullptr; DWORD n1 = 0;
    if (FAILED(m_pBuffer->Lock(0, pcmSize, &p1, &n1, nullptr, nullptr, 0))) {
        release();
        return false;
    }
    memcpy(p1, pcm, n1 < pcmSize ? n1 : pcmSize);
    m_pBuffer->Unlock(p1, n1, nullptr, 0);
    return true;
}

void CSound::release()
{
    if (m_pBuffer) { m_pBuffer->Release(); m_pBuffer = nullptr; }
}

bool CSound::play(int pan)
{
    if (!m_pBuffer) return false;

    // Only SetPan() is called here; the caller invokes Play() separately.
    // SetPan may reject an out-of-range stereo position, but playback still
    // happens with the previous pan value.
    m_pBuffer->SetPan(pan);
    m_pBuffer->SetCurrentPosition(0);

    HRESULT hr = m_pBuffer->Play(0, 0, 0);
    if (hr == DSERR_BUFFERLOST) {
        m_pBuffer->Restore();
        hr = m_pBuffer->Play(0, 0, 0);
    }
    return SUCCEEDED(hr);
}

void CSound::stop()
{
    if (m_pBuffer) m_pBuffer->Stop();
}

bool CMciMusic::open(const char* filename)
{
    if (!filename || !*filename) return false;
    stop();
    close();
    MCI_OPEN_PARMSA op;
    memset(&op, 0, sizeof(op));
    op.lpstrDeviceType = "sequencer";
    op.lpstrElementName = filename;
    if (mciSendCommandA(0, MCI_OPEN,
                        MCI_OPEN_TYPE | MCI_OPEN_ELEMENT | MCI_WAIT,
                        (DWORD_PTR)&op) != 0) {
        return false;
    }
    m_device = op.wDeviceID;
    return true;
}

bool CMciMusic::play()
{
    if (!m_device) return false;
    MCI_PLAY_PARMS pp;
    memset(&pp, 0, sizeof(pp));
    return mciSendCommandA(m_device, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)&pp) == 0;
}

bool CMciMusic::stop()
{
    if (!m_device) return true;
    return mciSendCommandA(m_device, MCI_STOP, 0, 0) == 0;
}

bool CMciMusic::close()
{
    if (!m_device) return true;
    MCIERROR e = mciSendCommandA(m_device, MCI_CLOSE, 0, 0);
    m_device = 0;
    return e == 0;
}
