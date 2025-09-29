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

#define WINDOW_NAME "DirectSound Secondary Buffer Play Stereo"

static BOOL TestDirectSoundBufferSingleWave(LPDIRECTSOUNDBUFFER a1, LPDIRECTSOUNDBUFFER a2, HWND wa,
    LPDIRECTSOUNDBUFFER b1, LPDIRECTSOUNDBUFFER b2, HWND wb, DWORD seconds,
    LPVOID wavea, DWORD wavea_length, LPVOID waveb, DWORD waveb_length) {
    if (a1 == NULL || a2 == NULL || b1 == NULL || b2 == NULL
        || wavea == NULL || wavea_length == 0 || waveb == NULL || waveb_length == 0) {
        return FALSE;
    }

    // GetCaps
    if (FAILED(CompareDirectSoundBufferCaps(a1, b1))) {
        return FALSE;
    }

    DWORD cpa = 0, cpb = 0, cwa = 0, cwb = 0;

    HRESULT ra = IDirectSoundBuffer_GetCurrentPosition(a1, &cpa, &cwa);
    HRESULT rb = IDirectSoundBuffer_GetCurrentPosition(b1, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        IDirectSoundBuffer_Stop(a1);
        IDirectSoundBuffer_Stop(b1);
    }

    if (ra != S_OK && rb != S_OK) {
        return TRUE;
    }

    ra = IDirectSoundBuffer_GetCurrentPosition(a2, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b2, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        IDirectSoundBuffer_Stop(a2);
        IDirectSoundBuffer_Stop(b2);
    }

    if (ra != S_OK && rb != S_OK) {
        return TRUE;
    }

    // Buffer 1
    {
        // Lock

        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a1, 0, 0, &a11, &al11, &a12, &al21, DSBLOCK_ENTIREBUFFER);
        rb = IDirectSoundBuffer_Lock(b1, 0, 0, &a21, &al12, &a22, &al22, DSBLOCK_ENTIREBUFFER);

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

        CopyMemory(a11, wavea, al11);
        CopyMemory(a21, wavea, al12);

        // Unlock

        ra = IDirectSoundBuffer_Unlock(a1, a11, al11, a12, al21);
        rb = IDirectSoundBuffer_Unlock(b1, a21, al12, a22, al22);

        if (ra != rb) {
            return FALSE;
        }
    }

    // Buffer 2
    {
        // Lock

        LPVOID a11 = NULL, a12 = NULL, a21 = NULL, a22 = NULL;
        DWORD al11 = 0, al12 = 0, al21 = 0, al22 = 0;

        ra = IDirectSoundBuffer_Lock(a2, 0, 0, &a11, &al11, &a12, &al21, DSBLOCK_ENTIREBUFFER);
        rb = IDirectSoundBuffer_Lock(b2, 0, 0, &a21, &al12, &a22, &al22, DSBLOCK_ENTIREBUFFER);

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

        CopyMemory(a11, waveb, al11);
        CopyMemory(a21, waveb, al12);

        // Unlock

        ra = IDirectSoundBuffer_Unlock(a2, a11, al11, a12, al21);
        rb = IDirectSoundBuffer_Unlock(b2, a21, al12, a22, al22);

        if (ra != rb) {
            return FALSE;
        }
    }

    // Play A
    ShowWindow(wa, SW_SHOW);
    UpdateWindow(wa);

    if (SUCCEEDED(ra = IDirectSoundBuffer_Play(a1, 0, 0, 0))) {
        IDirectSoundBuffer_Play(a2, 0, 0, 0);
        Sleep(seconds * 1000);
        IDirectSoundBuffer_Stop(a1);
        IDirectSoundBuffer_Stop(a2);
    }

    ShowWindow(wa, SW_HIDE);
    UpdateWindow(wa);

    // Play B
    ShowWindow(wb, SW_SHOW);
    UpdateWindow(wb);

    if (SUCCEEDED(rb = IDirectSoundBuffer_Play(b1, 0, 0, 0))) {
        IDirectSoundBuffer_Play(b2, 0, 0, 0);
        Sleep(seconds * 1000);
        IDirectSoundBuffer_Stop(b1);
        IDirectSoundBuffer_Stop(b2);
    }

    ShowWindow(wb, SW_HIDE);
    UpdateWindow(wb);

    if (ra != rb) {
        return FALSE;
    }

    ra = IDirectSoundBuffer_GetCurrentPosition(a1, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b1, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        IDirectSoundBuffer_Stop(a1);
        IDirectSoundBuffer_Stop(b1);
    }

    ra = IDirectSoundBuffer_GetCurrentPosition(a2, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b2, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        IDirectSoundBuffer_Stop(a2);
        IDirectSoundBuffer_Stop(b2);
    }

    return TRUE;
}

static BOOL TestDirectSoundBufferSecondaryPlayNotifyInstace(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb) {
    if (a == NULL || wa == NULL || b == NULL || wb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUND dsa = NULL, dsb = NULL;

    LPDIRECTSOUNDBUFFER dsba1 = NULL, dsba2 = NULL;
    LPDIRECTSOUNDBUFFER dsbb1 = NULL, dsbb2 = NULL;

    WAVEFORMATEX format;
    InitializeWaveFormat(&format, 2, 22050, 16);

    DSBUFFERDESC desc;
    InitializeDirectSoundBufferDesc(&desc, 0, 264600, &format);

    WAVEFORMATEX fa, fb;
    ZeroMemory(&fa, sizeof(WAVEFORMATEX));
    ZeroMemory(&fb, sizeof(WAVEFORMATEX));

    DWORD fas = 0, fbs = 0;

    LPVOID wavea = NULL, waveb = NULL;
    DWORD wavea_length = 0, waveb_length = 0;

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

    ra = IDirectSound_CreateSoundBuffer(dsa, &desc, &dsba1, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &desc, &dsbb1, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (dsba1 == NULL || dsbb1 == NULL) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSound_CreateSoundBuffer(dsa, &desc, &dsba2, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &desc, &dsbb2, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (dsba2 == NULL || dsbb2 == NULL) {
        result = FALSE;
        goto exit;
    }

    // GetCaps
    if (FAILED(CompareDirectSoundBufferCaps(dsba1, dsbb1))) {
        result = FALSE;
        goto exit;
    }

    // GetFormat

    ra = IDirectSoundBuffer_GetFormat(dsba1, &fa, sizeof(WAVEFORMATEX), &fas);
    rb = IDirectSoundBuffer_GetFormat(dsbb1, &fb, sizeof(WAVEFORMATEX), &fbs);

    if (ra != rb || fas != fbs) {
        result = FALSE;
        goto exit;
    }

    if (memcmp(&fa, &fb, sizeof(WAVEFORMATEX)) != 0) {
        result = FALSE;
        goto exit;
    }

    // Synthesise Wave

    if (!Synthesise(&fa, 440.0f, 3.0f /* seconds */, &wavea, &wavea_length)) {
        result = FALSE;
        goto exit;
    }

    if (!Synthesise(&fa, 550.0f, 3.0f /* seconds */, &waveb, &waveb_length)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundBufferSingleWave(dsba1, dsba2, wa, dsbb1, dsbb2, wb, 4, wavea, wavea_length, waveb, waveb_length)) {
        result = FALSE;
        goto exit;
    }

exit:

    if (wavea != NULL) {
        free(wavea);
    }

    if (waveb != NULL) {
        free(waveb);
    }

    RELEASE(dsba1);
    RELEASE(dsbb1);
    RELEASE(dsba2);
    RELEASE(dsbb2);
    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}

BOOL TestDirectSoundBufferSecondaryPlayMultiple(HMODULE a, HMODULE b) {
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

    if (!TestDirectSoundBufferSecondaryPlayNotifyInstace(dsca, wa, dscb, wb)) {
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
