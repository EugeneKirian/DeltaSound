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

#include "directsoundbuffer_secondary.h"
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

static BOOL TestDirectSoundBufferSingleWave(LPDIRECTSOUNDBUFFER a, HWND wa,
    LPDIRECTSOUNDBUFFER b, HWND wb, LPDIRECTSOUNDBUFFER ma, LPDIRECTSOUNDBUFFER mb,
    DWORD seconds, LPVOID wave, DWORD wave_length, DWORD priority, DWORD flags) {
    if (a == NULL || b == NULL || wave == NULL || wave_length == 0) {
        return FALSE;
    }

    // GetCaps
    if (FAILED(CompareDirectSoundBufferCaps(a, b))) {
        return FALSE;
    }

    DWORD cpa = 0, cpb = 0, cwa = 0, cwb = 0;

    HRESULT ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpa, &cwa);
    HRESULT rb = IDirectSoundBuffer_GetCurrentPosition(b, &cpb, &cwb);

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
        Sleep(seconds * 1000);
        IDirectSoundBuffer_Stop(a);
    }

    ShowWindow(wa, SW_HIDE);
    UpdateWindow(wa);

    // Play B
    ShowWindow(wb, SW_SHOW);
    UpdateWindow(wb);

    if (SUCCEEDED(rb = IDirectSoundBuffer_Play(b, 0, priority, flags))) {
        Sleep(seconds * 1000);
        IDirectSoundBuffer_Stop(b);
    }

    ShowWindow(wb, SW_HIDE);
    UpdateWindow(wb);

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

static BOOL TestDirectSoundBufferSecondaryPlaySingle(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb, DWORD dwFlags, DWORD dwLevel) {
    if (a == NULL || wa == NULL || b == NULL || wb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUND dsa = NULL, dsb = NULL;
    LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;
    LPDIRECTSOUNDBUFFER dsbma = NULL, dsbmb = NULL;

    WAVEFORMATEX format;
    InitializeWaveFormat(&format, 2, 22050, 8);

    DSBUFFERDESC desc;
    InitializeDirectSoundBufferDesc(&desc, dwFlags, 132300, &format);

    DSBUFFERDESC descm;
    InitializeDirectSoundBufferDesc(&descm, DSBCAPS_PRIMARYBUFFER, 0, NULL);

    WAVEFORMATEX fa, fb;
    ZeroMemory(&fa, sizeof(WAVEFORMATEX));
    ZeroMemory(&fb, sizeof(WAVEFORMATEX));

    DWORD fas = 0, fbs = 0;

    LPVOID wave = NULL;
    DWORD wave_length = 0;

    HRESULT ra = a(NULL, &dsa, NULL);
    HRESULT rb = b(NULL, &dsb, NULL);

    if (ra != rb) {
        return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        return FALSE;
    }

    ra = IDirectSound_SetCooperativeLevel(dsa, wa, dwLevel);
    rb = IDirectSound_SetCooperativeLevel(dsb, wb, dwLevel);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSound_CreateSoundBuffer(dsa, &descm, &dsbma, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsa, &descm, &dsbmb, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSound_CreateSoundBuffer(dsa, &desc, &dsba, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &desc, &dsbb, NULL);

    if (ra == E_NOTIMPL && (dwFlags & DSBCAPS_LOCHARDWARE)) {
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
    if (FAILED(CompareDirectSoundBufferCaps(dsba, dsbb))) {
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
        if (!TestDirectSoundBufferSingleWave(dsba, wa, dsbb, wb,
            dsbma, dsbmb, 4, wave, wave_length, 0, BufferPlayFlags[k])) {
            result = FALSE;
            goto exit;
        }
    }

exit:

    if (wave != NULL) {
        free(wave);
    }

    RELEASE(dsba);
    RELEASE(dsbb);
    RELEASE(dsbma);
    RELEASE(dsbmb);
    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}

BOOL TestDirectSoundBufferSecondaryPlay(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    if (!RegisterWindowClass(WINDOW_NAME)) {
        return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = GetDirectSoundCreate(a);
    LPDIRECTSOUNDCREATE dscb = GetDirectSoundCreate(b);

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
            if (!TestDirectSoundBufferSecondaryPlaySingle(dsca, wa, dscb, wb,
                BufferFlags[k], CooperativeLevels[i])) {
                result = FALSE;
                goto exit;
            }
        }
    }

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
