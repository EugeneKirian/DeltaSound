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

#include "directsoundbuffer_secondary_play.h"
#include "synth.h"
#include "wnd.h"

#define WINDOW_NAME "DirectSound Secondary Buffer Play"

#define BUFFER_FLAG_COUNT       17

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

static BOOL TestDirectSoundBufferSingleWave(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b,
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
    if (SUCCEEDED(ra = IDirectSoundBuffer_Play(a, 0, priority, flags))) {
        Sleep(1);
        // TODO check secondary buffer status
        // TODO check primary buffer status
        Sleep(seconds * 1000);
        IDirectSoundBuffer_Stop(a);
    }

    // Play B
    if (SUCCEEDED(rb = IDirectSoundBuffer_Play(b, 0, priority, flags))) {
        Sleep(1);
        // TODO check secondary buffer status
        // TODO check primary buffer status
        Sleep(seconds * 1000);
        IDirectSoundBuffer_Stop(b);
    }

    if (ra != rb) {
        return FALSE;
    }

    ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        IDirectSoundBuffer_Stop(a);
        IDirectSoundBuffer_Stop(b);
    }

    return TRUE;
}

static BOOL TestDirectSoundBufferSecondaryPlayOnce(
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
        goto exit;
    }

    ra = IDirectSound_CreateSoundBuffer(dsa, &desca, &dsba, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &descb, &dsbb, NULL);

    if (ra == E_NOTIMPL && (flags & DSBCAPS_LOCHARDWARE)) {
        goto exit;
    }

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (dsba == NULL || dsbb == NULL) {
        result = FALSE;
        goto exit;
    }

    // GetCaps

    ra = IDirectSoundBuffer_GetCaps(dsba, &capsa);
    rb = IDirectSoundBuffer_GetCaps(dsbb, &capsb);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (memcmp(&capsa, &capsb, sizeof(DSBCAPS)) != 0) {
        result = FALSE;
        goto exit;
    }

    // GetFormat

    ra = IDirectSoundBuffer_GetFormat(dsba, &fa, sizeof(WAVEFORMATEX), &fas);
    rb = IDirectSoundBuffer_GetFormat(dsbb, &fb, sizeof(WAVEFORMATEX), &fbs);

    if (ra != rb || fas != fbs) {
        result = FALSE;
        goto exit;
    }

    if (memcmp(&fa, &fb, sizeof(WAVEFORMATEX)) != 0) {
        result = FALSE;
        goto exit;
    }

    // Synthesise Wave

    if (!Synthesise(&fa, 293.66f, 3.0f /* seconds */, &wave, &wave_length)) {
        result = FALSE;
        goto exit;
    }

    for (int k = 0; k < BUFFER_PLAY_FLAGS_COUNT; k++) {
        if (!TestDirectSoundBufferSingleWave(dsba, dsbb, 4, wave, wave_length, 0, BufferPlayFlags[k])) {
            result = FALSE;
            goto exit;
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

BOOL TestDirectSoundBufferSecondaryPlay(HMODULE a, HMODULE b) {
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

    HWND wa = InitWindow(WINDOW_NAME);
    HWND wb = InitWindow(WINDOW_NAME);

    if (wa == NULL || wb == NULL) {
        result = FALSE;
        goto exit;
    }

    for (int i = 0; i < COOPERATIVE_LEVEL_COUNT; i++) {
        for (int k = 0; k < BUFFER_FLAG_COUNT; k++) {
            if (!TestDirectSoundBufferSecondaryPlayOnce(dsca, wa, dscb, wb,
                BufferFlags[k], CooperativeLevels[i])) {
                result = FALSE;
                goto exit;
            }
        }
    }

    // TODO play looping

exit:
    if (wa != NULL) {
        DestroyWindow(wa);
    }

    if (wb != NULL) {
        DestroyWindow(wb);
    }

    UnregisterClassA(WINDOW_NAME, GetModuleHandleA(NULL));

    return result;
}
