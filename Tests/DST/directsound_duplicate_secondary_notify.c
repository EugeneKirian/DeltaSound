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

#include "directsound_duplicate_secondary_notify.h"
#include "synth.h"
#include "wnd.h"

#define WINDOW_NAME "DirectSound Duplicate Secondary Buffer Play Notify"

#define TEST_EVENT_COUNT 7

typedef struct thread_context {
    BOOL    Run;
    DWORD   Count;
    HANDLE* Events;
    DWORD* Signal;
} thread_context;

static DWORD WINAPI NotifyThread(thread_context* ctx) {
    while (ctx->Run) {
        const DWORD result = WaitForMultipleObjects(ctx->Count, ctx->Events, FALSE, INFINITE);

        if (result < TEST_EVENT_COUNT) {
            ctx->Signal[result]++;
        }
    }

    return 0;
}

static BOOL TestDirectSoundBufferSingleWave(LPDIRECTSOUNDBUFFER a, HWND wa,
    LPDIRECTSOUNDBUFFER b, HWND wb, DWORD seconds, LPVOID wave, DWORD wave_length, DWORD dwFlags) {
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
    ShowWindow(wa, SW_SHOW);
    UpdateWindow(wa);

    if (SUCCEEDED(ra = IDirectSoundBuffer_Play(a, 0, 0, dwFlags))) {
        Sleep(seconds * 1000);
        IDirectSoundBuffer_Stop(a);
    }

    ShowWindow(wa, SW_HIDE);
    UpdateWindow(wa);

    // Play B
    ShowWindow(wb, SW_SHOW);
    UpdateWindow(wb);

    if (SUCCEEDED(rb = IDirectSoundBuffer_Play(b, 0, 0, dwFlags))) {
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

static BOOL TestDirectSoundBufferPlayNotify(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb, DWORD dwFlags) {
    if (a == NULL || wa == NULL || b == NULL || wb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    LPDIRECTSOUNDBUFFER dsba = NULL;
    LPDIRECTSOUNDBUFFER dsbb = NULL;

    LPDIRECTSOUNDBUFFER copya = NULL, copyb = NULL;

    const DWORD length = 144000;

    WAVEFORMATEX format;
    ZeroMemory(&format, sizeof(WAVEFORMATEX));

    format.wFormatTag = WAVE_FORMAT_PCM;
    format.nChannels = 2;
    format.nSamplesPerSec = 48000;
    format.nAvgBytesPerSec = 96000;
    format.nBlockAlign = 2;
    format.wBitsPerSample = 8;

    DSBUFFERDESC desca;
    ZeroMemory(&desca, sizeof(DSBUFFERDESC));

    desca.dwSize = sizeof(DSBUFFERDESC);
    desca.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY;
    desca.dwBufferBytes = length;
    desca.lpwfxFormat = &format;

    DSBUFFERDESC descb;
    ZeroMemory(&descb, sizeof(DSBUFFERDESC));

    descb.dwSize = sizeof(DSBUFFERDESC);
    descb.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY;
    descb.dwBufferBytes = length;
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

    HANDLE eventsa[TEST_EVENT_COUNT], eventsb[TEST_EVENT_COUNT];
    BOOL signala[TEST_EVENT_COUNT], signalb[TEST_EVENT_COUNT];

    thread_context ctxa, ctxb;

    ctxa.Count = TEST_EVENT_COUNT;
    ctxb.Count = TEST_EVENT_COUNT;

    ctxa.Run = TRUE;
    ctxb.Run = TRUE;

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

    ra = IDirectSound_CreateSoundBuffer(dsa, &desca, &dsba, NULL);
    rb = IDirectSound_CreateSoundBuffer(dsb, &descb, &dsbb, NULL);

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

    if (!Synthesise(&fa, 550.0f, 3.0f /* seconds */, &wave, &wave_length)) {
        result = FALSE;
        goto exit;
    }

    // Create Notifications
    for (int i = 0; i < TEST_EVENT_COUNT; i++) {
        eventsa[i] = CreateEventA(NULL, FALSE, FALSE, NULL);
        signala[i] = 0;

        eventsb[i] = CreateEventA(NULL, FALSE, FALSE, NULL);
        signalb[i] = 0;
    }

    ctxa.Events = eventsa;
    ctxa.Signal = signala;

    ctxb.Events = eventsb;
    ctxb.Signal = signalb;

    // Set Notifications
    {
        LPDIRECTSOUNDNOTIFY na = NULL;
        ra = IDirectSoundBuffer_QueryInterface(dsba, &IID_IDirectSoundNotify, &na);

        LPDIRECTSOUNDNOTIFY nb = NULL;
        rb = IDirectSoundBuffer_QueryInterface(dsbb, &IID_IDirectSoundNotify, &nb);

        if (ra != rb || na == NULL || nb == NULL) {
            result = FALSE;
            goto exit;
        }

        DSBPOSITIONNOTIFY pna[TEST_EVENT_COUNT];

        for (int i = 0; i < TEST_EVENT_COUNT; i++) {
            pna[i].dwOffset = i < (TEST_EVENT_COUNT - 1)
                ? i * (length / TEST_EVENT_COUNT) : DSBPN_OFFSETSTOP;
            pna[i].hEventNotify = eventsa[i];
        }

        ra = IDirectSoundNotify_SetNotificationPositions(na, TEST_EVENT_COUNT, pna);
        IDirectSoundNotify_Release(na);

        DSBPOSITIONNOTIFY pnb[TEST_EVENT_COUNT];

        for (int i = 0; i < TEST_EVENT_COUNT; i++) {
            pnb[i].dwOffset = i < (TEST_EVENT_COUNT - 1)
                ? i * (length / TEST_EVENT_COUNT) : DSBPN_OFFSETSTOP;
            pnb[i].hEventNotify = eventsb[i];
        }

        rb = IDirectSoundNotify_SetNotificationPositions(nb, TEST_EVENT_COUNT, pnb);
        IDirectSoundNotify_Release(nb);

        if (ra != rb) {
            result = FALSE;
            goto exit;
        }
    }

    // Duplicate
    {
        ra = IDirectSound_DuplicateSoundBuffer(dsa, dsba, &copya);
        rb = IDirectSound_DuplicateSoundBuffer(dsb, dsbb, &copyb);

        if (ra != rb) {
            result = FALSE;
            goto exit;
        }
    }

    HANDLE ha = CreateThread(NULL, 0, NotifyThread, &ctxa, 0, NULL);
    HANDLE hb = CreateThread(NULL, 0, NotifyThread, &ctxb, 0, NULL);

    if (!TestDirectSoundBufferSingleWave(copya, wa, copyb, wb, 4, wave, wave_length, dwFlags)) {
        result = FALSE;
        goto exit;
    }

    ctxa.Run = FALSE, ctxb.Run = FALSE;

    CloseHandle(ha);
    CloseHandle(hb);

    if (memcmp(signala, signalb, TEST_EVENT_COUNT * sizeof(BOOL)) != 0) {
        DWORD cpa = 0, cpb = 0;
        IDirectSoundBuffer_GetCurrentPosition(dsba, &cpa, NULL);
        IDirectSoundBuffer_GetCurrentPosition(dsbb, &cpb, NULL);

        goto exit;
    }

exit:

    for (int i = 0; i < TEST_EVENT_COUNT; i++) {
        CloseHandle(eventsa[i]);
        CloseHandle(eventsb[i]);
    }

    if (wave != NULL) {
        free(wave);
    }

    if (copya != NULL) {
        IDirectSoundBuffer_Release(copya);
    }

    if (copyb != NULL) {
        IDirectSoundBuffer_Release(copyb);
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

BOOL TestDirectSoundDuplicateSecondaryNotify(HMODULE a, HMODULE b) {
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

    if (!TestDirectSoundBufferPlayNotify(dsca, wa, dscb, wb, 0)) {
        result = FALSE;
        goto exit;
    }

    if (!TestDirectSoundBufferPlayNotify(dsca, wa, dscb, wb, DSBPLAY_LOOPING)) {
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
