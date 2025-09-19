/*
MIT License

Copyright (c) 2025 Eugene Kirian

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "directsound_duplicate_secondary.h"
#include "synth.h"
#include "wnd.h"

#define WINDOW_NAME "DirectSound Duplicate Secondary"

#define BUFFER_FLAG_COUNT   17

const static DWORD BufferFlags[BUFFER_FLAG_COUNT] = {
    0,
    DSBCAPS_STATIC,
    DSBCAPS_LOCHARDWARE,
    DSBCAPS_LOCSOFTWARE,
    DSBCAPS_CTRL3D,
    DSBCAPS_CTRLFREQUENCY,
    DSBCAPS_CTRLPAN,
    DSBCAPS_CTRLVOLUME,
    DSBCAPS_CTRL3D | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME,
    DSBCAPS_CTRL3D | DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY,
    DSBCAPS_CTRLPOSITIONNOTIFY,
    DSBCAPS_STICKYFOCUS,
    DSBCAPS_GLOBALFOCUS,
    DSBCAPS_STICKYFOCUS | DSBCAPS_GLOBALFOCUS,
    DSBCAPS_GETCURRENTPOSITION2,
    DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE,
    DSBCAPS_TRUEPLAYPOSITION
};

static BOOL TestDirectSoundDuplicateSoundBuffer(LPDIRECTSOUND a, LPDIRECTSOUND b, DWORD flags) {
    BOOL result = TRUE;
    LPDIRECTSOUNDBUFFER dsba = NULL;
    LPDIRECTSOUNDBUFFER dsbb = NULL;

    LPDIRECTSOUNDBUFFER dsbac = NULL;
    LPDIRECTSOUNDBUFFER dsbbc = NULL;

    WAVEFORMATEX format;
    ZeroMemory(&format, sizeof(WAVEFORMATEX));

    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = 22050;
    format.nAvgBytesPerSec = 44100;
    format.nBlockAlign = 2;
    format.wBitsPerSample = 8;

    DSBUFFERDESC desca;
    ZeroMemory(&desca, sizeof(DSBUFFERDESC));

    desca.dwSize = sizeof(DSBUFFERDESC);
    desca.dwFlags = flags;
    desca.dwBufferBytes = 4 * format.nAvgBytesPerSec;
    desca.lpwfxFormat = &format;

    DSBUFFERDESC descb;
    ZeroMemory(&descb, sizeof(DSBUFFERDESC));

    descb.dwSize = sizeof(DSBUFFERDESC);
    descb.dwFlags = flags;
    descb.dwBufferBytes = 4 * format.nAvgBytesPerSec;
    descb.lpwfxFormat = &format;

    DSBCAPS capsa;
    ZeroMemory(&capsa, sizeof(DSBCAPS));
    capsa.dwSize = sizeof(DSBCAPS);

    DSBCAPS capsb;
    ZeroMemory(&capsb, sizeof(DSBCAPS));
    capsb.dwSize = sizeof(DSBCAPS);

    DWORD cpa = 0, cwa = 0, cpb = 0, cwb = 0;

    DWORD fa = 0, fb = 0;
    DWORD va = 0, vb = 0;
    DWORD pa = 0, pb = 0;

    HRESULT ra = IDirectSound_CreateSoundBuffer(a, &desca, &dsba, NULL);
    HRESULT rb = IDirectSound_CreateSoundBuffer(b, &descb, &dsbb, NULL);

    if (dsba == NULL || dsbb == NULL) {
        if (flags & DSBCAPS_LOCHARDWARE) {
            goto exit;
        }

        result = FALSE;
        DebugBreak(); goto exit;
    }

    ra = IDirectSound_DuplicateSoundBuffer(a, dsba, NULL);
    rb = IDirectSound_DuplicateSoundBuffer(b, dsbb, NULL);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    ra = IDirectSound_DuplicateSoundBuffer(a, NULL, NULL);
    rb = IDirectSound_DuplicateSoundBuffer(b, NULL, NULL);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    ra = IDirectSound_DuplicateSoundBuffer(a, NULL, &dsbac);
    rb = IDirectSound_DuplicateSoundBuffer(b, NULL, &dsbbc);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    // SetFrequency

    ra = IDirectSoundBuffer_SetFrequency(dsba, 96000);
    rb = IDirectSoundBuffer_SetFrequency(dsbb, 96000);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    // SetPan

    ra = IDirectSoundBuffer_SetPan(dsba, DSBPAN_LEFT);
    rb = IDirectSoundBuffer_SetPan(dsbb, DSBPAN_LEFT);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    // SetVolume

    ra = IDirectSoundBuffer_SetVolume(dsba, DSBVOLUME_MIN / 2);
    rb = IDirectSoundBuffer_SetVolume(dsbb, DSBVOLUME_MIN / 2);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    // SetCurrentPosition

    ra = IDirectSoundBuffer_SetCurrentPosition(dsba, 10000);
    rb = IDirectSoundBuffer_SetCurrentPosition(dsbb, 10000);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    // Duplicate

    ra = IDirectSound_DuplicateSoundBuffer(a, dsba, &dsbac);
    rb = IDirectSound_DuplicateSoundBuffer(b, dsbb, &dsbbc);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (ra != S_OK || rb != S_OK) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    // GetCaps

    ra = IDirectSoundBuffer_GetCaps(dsbac, &capsa);
    rb = IDirectSoundBuffer_GetCaps(dsbbc, &capsb);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (memcmp(&capsa, &capsb, sizeof(DSBCAPS)) != 0) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    // GetCurrentPosition

    ra = IDirectSoundBuffer_GetCurrentPosition(dsbac, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(dsbbc, &cpb, &cwa);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (cpa != cpb || cwa != cwa) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    // GetFrequency

    ra = IDirectSoundBuffer_GetFrequency(dsbac, &fa);
    rb = IDirectSoundBuffer_GetFrequency(dsbbc, &fb);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (pa != pb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    // GetPan

    ra = IDirectSoundBuffer_GetPan(dsbac, &pa);
    rb = IDirectSoundBuffer_GetPan(dsbbc, &pb);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (pa != pb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    // GetVolume

    ra = IDirectSoundBuffer_GetVolume(dsbac, &va);
    rb = IDirectSoundBuffer_GetVolume(dsbbc, &vb);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (va != vb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

exit:

    if (dsbac != NULL) {
        IDirectSoundBuffer_Release(dsbac);
    }

    if (dsbbc != NULL) {
        IDirectSoundBuffer_Release(dsbbc);
    }

    if (dsba != NULL) {
        IDirectSoundBuffer_Release(dsba);
    }

    if (dsbb != NULL) {
        IDirectSoundBuffer_Release(dsbb);
    }

    return result;
}

// TODO refactor almost complete copy of
// TestDirectSoundBufferSecondaryPlaySingle
// from directsoundbuffer_secondary_play.c
// and buffer duplication
static BOOL TestDirectSoundBufferSingleWave(
    LPDIRECTSOUND dsa, LPDIRECTSOUND dsb,
    LPDIRECTSOUNDBUFFER a, HWND wa,
    LPDIRECTSOUNDBUFFER b, HWND wb,
    DWORD seconds, LPVOID wave, DWORD wave_length, DWORD priority, DWORD flags) {
    if (a == NULL || b == NULL || wave == NULL || wave_length == 0) {
        return FALSE;
    }

    // GetCaps
    DSBCAPS capsa;
    ZeroMemory(&capsa, sizeof(DSBCAPS));
    capsa.dwSize = sizeof(DSBCAPS);

    DSBCAPS capsb;
    ZeroMemory(&capsb, sizeof(DSBCAPS));
    capsb.dwSize = sizeof(DSBCAPS);

    LPDIRECTSOUNDBUFFER copya = NULL;
    LPDIRECTSOUNDBUFFER copyb = NULL;
    DWORD stata = 0, statb = 0;
    DWORD statca = 0, statcb = 0;
    HRESULT rca = S_OK, rcb = S_OK;
    DWORD pcpa = 0, pcpb = 0, pcwa = 0, pcwb = 0;
    DWORD pcpac = 0, pcpbc = 0, pcwac = 0, pcwbc = 0;

    HRESULT ra = IDirectSoundBuffer_GetCaps(a, &capsa);
    HRESULT rb = IDirectSoundBuffer_GetCaps(b, &capsb);

    if (ra != rb) {
        return FALSE;
    }

    if (memcmp(&capsa, &capsb, sizeof(DSBCAPS)) != 0) {
        return FALSE;
    }

    DWORD cpa = 0, cpb = 0, cwa = 0, cwb = 0;

    ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        IDirectSoundBuffer_Stop(a);
        IDirectSoundBuffer_Stop(b);
    }

    if (ra != S_OK && rb != S_OK) {
        return TRUE;
    }

    // Lock

    LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
    DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

    ra = IDirectSoundBuffer_Lock(a, 0, 0, &a11, &al11, &a12, &al21, DSBLOCK_ENTIREBUFFER);
    rb = IDirectSoundBuffer_Lock(b, 0, 0, &a21, &al12, &a22, &al22, DSBLOCK_ENTIREBUFFER);

    if (ra != rb) {
        return FALSE;
    }

    if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
        return FALSE;
    }

    if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
        return FALSE;
    }

    if (ra != S_OK && rb != S_OK) {
        return TRUE;
    }

    // Copy

    if (a11 == NULL || a21 == NULL) {
        return FALSE;
    }

    CopyMemory(a11, wave, al11);
    CopyMemory(a21, wave, al12);

    // Unlock

    ra = IDirectSoundBuffer_Unlock(a, a11, al11, a12, al21);
    rb = IDirectSoundBuffer_Unlock(b, a21, al12, a22, al22);

    if (ra != rb) {
        return FALSE;
    }

    // Play A
    ShowWindow(wa, SW_SHOW);
    UpdateWindow(wa);

    if (SUCCEEDED(ra = IDirectSoundBuffer_Play(a, 0, priority, flags))) {
        Sleep(100);
        IDirectSoundBuffer_GetStatus(a, &stata);
        IDirectSoundBuffer_GetCurrentPosition(a, &pcpa, &pcwa);

        rca = IDirectSound_DuplicateSoundBuffer(dsa, a, &copya);

        if (copya != NULL) {
            IDirectSoundBuffer_GetStatus(copya, &statca);
            IDirectSoundBuffer_GetCurrentPosition(copya, &pcpac, &pcwac);
        }

        Sleep(seconds * 1000);
        IDirectSoundBuffer_Stop(a);
    }

    ShowWindow(wa, SW_HIDE);
    UpdateWindow(wa);

    // Play B
    ShowWindow(wb, SW_SHOW);
    UpdateWindow(wb);

    if (SUCCEEDED(rb = IDirectSoundBuffer_Play(b, 0, priority, flags))) {
        Sleep(100);
        IDirectSoundBuffer_GetStatus(b, &statb);
        IDirectSoundBuffer_GetCurrentPosition(b, &pcpb, &pcwb);

        rcb = IDirectSound_DuplicateSoundBuffer(dsb, b, &copyb);

        if (copyb != NULL) {
            IDirectSoundBuffer_GetStatus(copyb, &statcb);
            IDirectSoundBuffer_GetCurrentPosition(copyb, &pcpbc, &pcwbc);
        }

        Sleep(seconds * 1000);
        IDirectSoundBuffer_Stop(b);
    }

    ShowWindow(wb, SW_HIDE);
    UpdateWindow(wb);

    BOOL result = TRUE;

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (rca != rcb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if ((copya == NULL && copyb != NULL)
        || (copya != NULL && copyb == NULL)) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (copya != NULL && copyb != NULL) {
        if (stata != statb || stata == statca || statb == statcb || statca != statcb) {
            result = FALSE;
            DebugBreak(); goto exit;
        }

        if (pcpa == pcpac || pcwb == pcwbc) {
            result = FALSE;
            DebugBreak(); goto exit;
        }
    }

    ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (cpa != cpb || cwa != cwb) {
        IDirectSoundBuffer_Stop(a);
        IDirectSoundBuffer_Stop(b);
    }

exit:

    if (copya != NULL) {
        IDirectSoundBuffer_Release(copya);
    }

    if (copyb != NULL) {
        IDirectSoundBuffer_Release(copyb);
    }

    return result;
}

// TODO refactor almost complete copy of
// TestDirectSoundBufferSecondaryPlaySingle
// from directsoundbuffer_secondary_play.c
// and buffer duplication
static BOOL TestDirectSoundBufferSecondaryPlaySingle(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb, DWORD flags, DWORD level) {
    if (a == NULL || wa == NULL || b == NULL || wb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    LPDIRECTSOUNDBUFFER dsba = NULL;
    LPDIRECTSOUNDBUFFER dsbb = NULL;

    WAVEFORMATEX format;
    ZeroMemory(&format, sizeof(WAVEFORMATEX));

    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = 22050;
    format.nAvgBytesPerSec = 44100;
    format.nBlockAlign = 2;
    format.wBitsPerSample = 8;

    DSBUFFERDESC desca;
    ZeroMemory(&desca, sizeof(DSBUFFERDESC));

    desca.dwSize = sizeof(DSBUFFERDESC);
    desca.dwFlags = flags;
    desca.dwBufferBytes = 132300;
    desca.lpwfxFormat = &format;

    DSBUFFERDESC descb;
    ZeroMemory(&descb, sizeof(DSBUFFERDESC));

    descb.dwSize = sizeof(DSBUFFERDESC);
    descb.dwFlags = flags;
    descb.dwBufferBytes = 132300;
    descb.lpwfxFormat = &format;

    WAVEFORMATEX fa;
    ZeroMemory(&fa, sizeof(WAVEFORMATEX));

    WAVEFORMATEX fb;
    ZeroMemory(&fb, sizeof(WAVEFORMATEX));
    DWORD fas = 0, fbs = 0;

    LPVOID wave = NULL;
    DWORD wave_length = 0;

    DSBCAPS capsa;
    ZeroMemory(&capsa, sizeof(DSBCAPS));
    capsa.dwSize = sizeof(DSBCAPS);

    DSBCAPS capsb;
    ZeroMemory(&capsb, sizeof(DSBCAPS));
    capsb.dwSize = sizeof(DSBCAPS);

    HRESULT ra = a(NULL, &dsa, NULL);
    HRESULT rb = b(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        return FALSE;
    }

    ra = IDirectSound_SetCooperativeLevel(dsa, wa, level);
    rb = IDirectSound_SetCooperativeLevel(dsb, wb, level);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    ra = IDirectSound_CreateSoundBuffer(dsa, &desca, &dsba, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &descb, &dsbb, NULL);

    if (ra == E_NOTIMPL && (flags & DSBCAPS_LOCHARDWARE)) {
        goto exit;
    }

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (dsba == NULL || dsbb == NULL) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    // GetCaps

    ra = IDirectSoundBuffer_GetCaps(dsba, &capsa);
    rb = IDirectSoundBuffer_GetCaps(dsbb, &capsb);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (memcmp(&capsa, &capsb, sizeof(DSBCAPS)) != 0) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    // GetFormat

    ra = IDirectSoundBuffer_GetFormat(dsba, &fa, sizeof(WAVEFORMATEX), &fas);
    rb = IDirectSoundBuffer_GetFormat(dsbb, &fb, sizeof(WAVEFORMATEX), &fbs);

    if (ra != rb || fas != fbs) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (memcmp(&fa, &fb, sizeof(WAVEFORMATEX)) != 0) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    // Synthesise Wave

    if (!Synthesise(&fa, 293.66f, 3.0f /* seconds */, &wave, &wave_length)) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    for (int k = 0; k < BUFFER_PLAY_FLAGS_COUNT; k++) {
        if (!TestDirectSoundBufferSingleWave(dsa, dsb, dsba, wa, dsbb, wb,
            4, wave, wave_length, 0, BufferPlayFlags[k])) {
            result = FALSE;
            DebugBreak(); goto exit;
        }
    }

exit:

    if (wave != NULL) {
        free(wave);
    }

    if (dsba != NULL) {
        IDirectSoundBuffer_Release(dsba);
    }

    if (dsbb != NULL) {
        IDirectSoundBuffer_Release(dsbb);
    }

    if (dsa != NULL) {
        IDirectSound_Release(dsa);
    }

    if (dsb != NULL) {
        IDirectSound_Release(dsb);
    }

    return result;
}

BOOL TestDirectSoundDuplicateSecondary(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    if (!RegisterWindowClass(WINDOW_NAME)) {
        return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = (LPDIRECTSOUNDCREATE)GetProcAddress(a, "DirectSoundCreate");
    LPDIRECTSOUNDCREATE dscb = (LPDIRECTSOUNDCREATE)GetProcAddress(b, "DirectSoundCreate");

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    HRESULT ra = dsca(NULL, &dsa, NULL);
    HRESULT rb = dscb(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        return FALSE;
    }

    HWND wa = InitWindow(WINDOW_NAME);
    HWND wb = InitWindow(WINDOW_NAME);

    if (wa == NULL || wb == NULL) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    for (int i = 0; i < BUFFER_FLAG_COUNT; i++) {
        if (!TestDirectSoundDuplicateSoundBuffer(dsa, dsb, BufferFlags[i])) {
            result = FALSE;
            DebugBreak(); goto exit;
        }
    }


    // TODO refactor!
    // Copy from directsoundbuffer_secondary_play.c
    for (int i = 0; i < COOPERATIVE_LEVEL_COUNT; i++) {
        for (int k = 0; k < BUFFER_FLAG_COUNT; k++) {
            if (!TestDirectSoundBufferSecondaryPlaySingle(dsca, wa, dscb, wb,
                BufferFlags[k], CooperativeLevels[i])) {
                result = FALSE;
                DebugBreak(); goto exit;
            }
        }
    }


    // TODO test while playing, check priority, status, etc...
    // TODO in what state buffer is created if the original is playing?
    // TODO what happens to the flags?
    // TODO todo what happens with notifications?

exit:

    IDirectSound_Release(dsa);
    IDirectSound_Release(dsb);

    if (wa != NULL) {
        DestroyWindow(wa);
    }

    if (wb != NULL) {
        DestroyWindow(wb);
    }

    UnregisterClassA(WINDOW_NAME, GetModuleHandleA(NULL));

    return result;
}
