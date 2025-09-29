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

#define WINDOW_NAME "DirectSound Secondary Buffer Play Frequency"

static BOOL TestDirectSoundBufferSingleWave(LPDIRECTSOUNDBUFFER a, HWND wa,
    LPDIRECTSOUNDBUFFER b, HWND wb, DWORD seconds, LPVOID wave, DWORD wave_length) {
    if (a == NULL || b == NULL || wave == NULL || wave_length == 0) {
        return FALSE;
    }

    // GetCaps
    if (FAILED(CompareDirectSoundBufferCaps(a, b))) {
        return FALSE;
    }

    WAVEFORMATEX fa, fb;
    ZeroMemory(&fa, sizeof(WAVEFORMATEX));
    ZeroMemory(&fb, sizeof(WAVEFORMATEX));


    DWORD fas = 0, fbs = 0;

    HRESULT ra = IDirectSoundBuffer_GetFormat(a, &fa, sizeof(WAVEFORMATEX), &fas);
    HRESULT rb = IDirectSoundBuffer_GetFormat(b, &fb, sizeof(WAVEFORMATEX), &fbs);

    if (ra != rb) {
        return FALSE;
    }

    if (memcmp(&fa, &fb, sizeof(WAVEFORMATEX)) != 0) {
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

    if (SUCCEEDED(ra = IDirectSoundBuffer_Play(a, 0, 0, 0))) {
        for (DWORD i = 0; i < seconds; i++) {
            const DWORD frequency =
                fa.nSamplesPerSec + ((i % 2 == 0) ? 1 : -1) * (fa.nSamplesPerSec / (seconds + i));

            IDirectSoundBuffer_SetFrequency(a, frequency);

            Sleep(1000);
        }

        IDirectSoundBuffer_Stop(a);
    }

    ShowWindow(wa, SW_HIDE);
    UpdateWindow(wa);

    // Play B
    ShowWindow(wb, SW_SHOW);
    UpdateWindow(wb);

    if (SUCCEEDED(ra = IDirectSoundBuffer_Play(b, 0, 0, 0))) {
        for (DWORD i = 0; i < seconds; i++) {
            const DWORD frequency =
                fa.nSamplesPerSec + ((i % 2 == 0) ? 1 : -1) * (fa.nSamplesPerSec / (seconds + i));

            IDirectSoundBuffer_SetFrequency(b, frequency);

            Sleep(1000);
        }

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

static BOOL TestDirectSoundBufferSecondaryPlaySingleFrequency(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb) {
    if (a == NULL || wa == NULL || b == NULL || wb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUND dsa = NULL, dsb = NULL;
    LPDIRECTSOUNDBUFFER dsba = NULL, dsbb = NULL;

    WAVEFORMATEX format;
    InitializeWaveFormat(&format, 2, 48000, 8);

    DSBUFFERDESC desc;
    InitializeDirectSoundBufferDesc(&desc,
        DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY, 144000, &format);

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

    ra = IDirectSound_SetCooperativeLevel(dsa, wa, DSSCL_NORMAL);
    rb = IDirectSound_SetCooperativeLevel(dsb, wb, DSSCL_NORMAL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSound_CreateSoundBuffer(dsa, &desc, &dsba, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &desc, &dsbb, NULL);

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

    if (!Synthesise(&fa, 550.0f, 3.0f /* seconds */, &wave, &wave_length)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundBufferSingleWave(dsba, wa, dsbb, wb, 4, wave, wave_length)) {
        result = FALSE;
        goto exit;
    }

exit:

    if (wave != NULL) {
        free(wave);
    }

    RELEASE(dsba);
    RELEASE(dsbb);
    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}

BOOL TestDirectSoundBufferSecondaryPlayFrequency(HMODULE a, HMODULE b) {
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

    if (!TestDirectSoundBufferSecondaryPlaySingleFrequency(dsca, wa, dscb, wb)) {
        result = FALSE;
        goto exit;
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
