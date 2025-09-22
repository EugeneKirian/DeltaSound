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

#include "directsoundbuffer_secondary_play_stereo.h"
#include "synth.h"
#include "wnd.h"

#define WINDOW_NAME "DirectSound Secondary Buffer Play Stereo"

static BOOL TestDirectSoundBufferSingleWave(LPDIRECTSOUNDBUFFER a1, LPDIRECTSOUNDBUFFER a2, HWND wa,
    LPDIRECTSOUNDBUFFER b1, LPDIRECTSOUNDBUFFER b2, HWND wb, DWORD seconds,
    LPVOID wavea, DWORD wavea_length, LPVOID waveb, DWORD waveb_length) {
    if (a1 == NULL || a2 == NULL || b1 == NULL || b2 == NULL
        || wavea == NULL || wavea_length == 0 || waveb == NULL || waveb_length == 0) {
        DebugBreak(); return FALSE;
    }

    // GetCaps
    DSBCAPS capsa;
    ZeroMemory(&capsa, sizeof(DSBCAPS));
    capsa.dwSize = sizeof(DSBCAPS);

    DSBCAPS capsb;
    ZeroMemory(&capsb, sizeof(DSBCAPS));
    capsb.dwSize = sizeof(DSBCAPS);

    HRESULT ra = IDirectSoundBuffer_GetCaps(a1, &capsa);
    HRESULT rb = IDirectSoundBuffer_GetCaps(b1, &capsb);

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    if (memcmp(&capsa, &capsb, sizeof(DSBCAPS)) != 0) {
        DebugBreak(); return FALSE;
    }

    DWORD cpa = 0, cpb = 0, cwa = 0, cwb = 0;

    ra = IDirectSoundBuffer_GetCurrentPosition(a1, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b1, &cpb, &cwb);

    if (ra != rb) {
        DebugBreak(); return FALSE;
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
        DebugBreak(); return FALSE;
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
            DebugBreak(); return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            DebugBreak(); return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            DebugBreak(); return FALSE;
        }

        if (ra != S_OK && rb != S_OK) {
            return TRUE;
        }

        // Copy

        if (a11 == NULL || a21 == NULL) {
            DebugBreak(); return FALSE;
        }

        CopyMemory(a11, wavea, al11);
        CopyMemory(a21, wavea, al12);

        // Unlock

        ra = IDirectSoundBuffer_Unlock(a1, a11, al11, a12, al21);
        rb = IDirectSoundBuffer_Unlock(b1, a21, al12, a22, al22);

        if (ra != rb) {
            DebugBreak(); return FALSE;
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
            DebugBreak(); return FALSE;
        }

        if ((a11 == NULL && a21 != NULL) || (a11 != NULL && a21 == NULL)) {
            DebugBreak(); return FALSE;
        }

        if ((a12 == NULL && a22 != NULL) || (a12 != NULL && a22 == NULL)) {
            DebugBreak(); return FALSE;
        }

        if (ra != S_OK && rb != S_OK) {
            return TRUE;
        }

        // Copy

        if (a11 == NULL || a21 == NULL) {
            DebugBreak(); return FALSE;
        }

        CopyMemory(a11, waveb, al11);
        CopyMemory(a21, waveb, al12);

        // Unlock

        ra = IDirectSoundBuffer_Unlock(a2, a11, al11, a12, al21);
        rb = IDirectSoundBuffer_Unlock(b2, a21, al12, a22, al22);

        if (ra != rb) {
            DebugBreak(); return FALSE;
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
        DebugBreak(); return FALSE;
    }

    ra = IDirectSoundBuffer_GetCurrentPosition(a1, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b1, &cpb, &cwb);

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        IDirectSoundBuffer_Stop(a1);
        IDirectSoundBuffer_Stop(b1);
    }

    ra = IDirectSoundBuffer_GetCurrentPosition(a2, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(b2, &cpb, &cwb);

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        IDirectSoundBuffer_Stop(a2);
        IDirectSoundBuffer_Stop(b2);
    }

    return TRUE;
}

static BOOL TestDirectSoundBufferSecondaryPlaySingleMultiple(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb) {
    if (a == NULL || wa == NULL || b == NULL || wb == NULL) {
        DebugBreak(); return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    LPDIRECTSOUNDBUFFER dsba1 = NULL, dsba2 = NULL;
    LPDIRECTSOUNDBUFFER dsbb1 = NULL, dsbb2 = NULL;

    WAVEFORMATEX format;
    ZeroMemory(&format, sizeof(WAVEFORMATEX));

    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = 22050;
    format.nAvgBytesPerSec = 88200;
    format.nBlockAlign = 4;
    format.wBitsPerSample = 16;

    DSBUFFERDESC desca;
    ZeroMemory(&desca, sizeof(DSBUFFERDESC));

    desca.dwSize = sizeof(DSBUFFERDESC);
    desca.dwBufferBytes = 264600;
    desca.lpwfxFormat = &format;

    DSBUFFERDESC descb;
    ZeroMemory(&descb, sizeof(DSBUFFERDESC));

    descb.dwSize = sizeof(DSBUFFERDESC);
    descb.dwBufferBytes = 264600;
    descb.lpwfxFormat = &format;

    WAVEFORMATEX fa;
    ZeroMemory(&fa, sizeof(WAVEFORMATEX));

    WAVEFORMATEX fb;
    ZeroMemory(&fb, sizeof(WAVEFORMATEX));
    DWORD fas = 0, fbs = 0;

    LPVOID wavea = NULL, waveb = NULL;
    DWORD wavea_length = 0, waveb_length = 0;

    DSBCAPS capsa;
    ZeroMemory(&capsa, sizeof(DSBCAPS));
    capsa.dwSize = sizeof(DSBCAPS);

    DSBCAPS capsb;
    ZeroMemory(&capsb, sizeof(DSBCAPS));
    capsb.dwSize = sizeof(DSBCAPS);

    HRESULT ra = a(NULL, &dsa, NULL);
    HRESULT rb = b(NULL, &dsb, NULL);

    if (ra != rb) {
        DebugBreak(); return FALSE;
    }

    if (dsa == NULL || dsb == NULL) {
        DebugBreak(); return FALSE;
    }

    ra = IDirectSound_SetCooperativeLevel(dsa, wa, DSSCL_NORMAL);
    rb = IDirectSound_SetCooperativeLevel(dsb, wb, DSSCL_NORMAL);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    ra = IDirectSound_CreateSoundBuffer(dsa, &desca, &dsba1, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &descb, &dsbb1, NULL);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (dsba1 == NULL || dsbb1 == NULL) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    ra = IDirectSound_CreateSoundBuffer(dsa, &desca, &dsba2, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &descb, &dsbb2, NULL);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (dsba2 == NULL || dsbb2 == NULL) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    // GetCaps

    ra = IDirectSoundBuffer_GetCaps(dsba1, &capsa);
    rb = IDirectSoundBuffer_GetCaps(dsbb1, &capsb);

    if (ra != rb) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (memcmp(&capsa, &capsb, sizeof(DSBCAPS)) != 0) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    // GetFormat

    ra = IDirectSoundBuffer_GetFormat(dsba1, &fa, sizeof(WAVEFORMATEX), &fas);
    rb = IDirectSoundBuffer_GetFormat(dsbb1, &fb, sizeof(WAVEFORMATEX), &fbs);

    if (ra != rb || fas != fbs) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (memcmp(&fa, &fb, sizeof(WAVEFORMATEX)) != 0) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    // Synthesise Wave

    if (!Synthesise(&fa, 440.0f, 3.0f /* seconds */, &wavea, &wavea_length)) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (!Synthesise(&fa, 550.0f, 3.0f /* seconds */, &waveb, &waveb_length)) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (!TestDirectSoundBufferSingleWave(dsba1, dsba2, wa, dsbb1, dsbb2, wb, 4, wavea, wavea_length, waveb, waveb_length)) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

exit:

    if (wavea != NULL) {
        free(wavea);
    }
    
    if (waveb != NULL) {
        free(waveb);
    }

    if (dsba1 != NULL) {
        IDirectSoundBuffer_Release(dsba1);
    }

    if (dsbb1 != NULL) {
        IDirectSoundBuffer_Release(dsbb1);
    }

    if (dsba2 != NULL) {
        IDirectSoundBuffer_Release(dsba2);
    }

    if (dsbb2 != NULL) {
        IDirectSoundBuffer_Release(dsbb2);
    }

    if (dsa != NULL) {
        IDirectSound_Release(dsa);
    }

    if (dsb != NULL) {
        IDirectSound_Release(dsb);
    }

    return result;
}

BOOL TestDirectSoundBufferSecondaryPlayMultiple(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        DebugBreak(); return FALSE;
    }

    if (!RegisterWindowClass(WINDOW_NAME)) {
        DebugBreak(); return FALSE;
    }

    LPDIRECTSOUNDCREATE dsca = (LPDIRECTSOUNDCREATE)GetProcAddress(a, "DirectSoundCreate");
    LPDIRECTSOUNDCREATE dscb = (LPDIRECTSOUNDCREATE)GetProcAddress(b, "DirectSoundCreate");

    if (dsca == NULL || dscb == NULL) {
        DebugBreak(); return FALSE;
    }

    BOOL result = TRUE;

    HWND wa = InitWindow(WINDOW_NAME);
    HWND wb = InitWindow(WINDOW_NAME);

    if (wa == NULL || wb == NULL) {
        result = FALSE;
        DebugBreak(); goto exit;
    }

    if (!TestDirectSoundBufferSecondaryPlaySingleMultiple(dsca, wa, dscb, wb)) {
        result = FALSE;
        DebugBreak(); goto exit;
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
