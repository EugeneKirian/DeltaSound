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

#include "directsoundcapturebuffer.h"

#define TEST_EVENT_COUNT 7

typedef struct thread_context {
    BOOL    Run;
    DWORD   Count;
    HANDLE* Events;
    DWORD*  Signal;
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

static BOOL TestDirectSoundCaptureBufferReleaseNotify(LPDIRECTSOUNDCAPTURECREATE a, LPDIRECTSOUNDCAPTURECREATE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;
    LPDIRECTSOUNDCAPTUREBUFFER dsba = NULL, dsbb = NULL;
    LPDIRECTSOUNDCAPTUREBUFFER dsbac = NULL, dsbbc = NULL;
    LPDIRECTSOUNDNOTIFY dsna = NULL, dsnb = NULL;
    DWORD rca = 0, rcb = 0;

    const DWORD length = 144000;

    WAVEFORMATEX format;
    InitializeWaveFormat(&format, 2, 48000, 8);

    DSCBUFFERDESC desc;
    InitializeDirectSoundCaptureBufferDesc(&desc, 0, length, &format);

    HRESULT ra = a(NULL, &dsa, NULL);
    HRESULT rb = b(NULL, &dsb, NULL);

    ra = IDirectSoundCapture_CreateCaptureBuffer(dsa, &desc, &dsba, NULL);
    rb = IDirectSoundCapture_CreateCaptureBuffer(dsb, &desc, &dsbb, NULL);

    if (ra != rb || dsba == NULL || dsbb == NULL) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSoundCaptureBuffer_QueryInterface(dsba, &IID_IDirectSoundNotify, &dsna);
    rb = IDirectSoundCaptureBuffer_QueryInterface(dsbb, &IID_IDirectSoundNotify, &dsnb);

    if (ra != rb || dsna == NULL || dsnb == NULL) {
        result = FALSE;
        goto exit;
    }

    rca = IDirectSoundCaptureBuffer_Release(dsba);
    rcb = IDirectSoundCaptureBuffer_Release(dsbb);

    if (rca != rcb || rca != 0) {
        result = FALSE;
        goto exit;
    }

    ra = IDirectSoundCapture_CreateCaptureBuffer(dsa, &desc, &dsbac, NULL);
    rb = IDirectSoundCapture_CreateCaptureBuffer(dsb, &desc, &dsbbc, NULL);

    if (ra != rb || dsbac != NULL || dsbbc != NULL) {
        result = FALSE;
        goto exit;
    }

    dsbac = dsba;
    dsbbc = dsbb;

    ra = IDirectSoundNotify_QueryInterface(dsna, &IID_IDirectSoundCaptureBuffer, &dsba);
    rb = IDirectSoundNotify_QueryInterface(dsnb, &IID_IDirectSoundCaptureBuffer, &dsbb);

    if (ra != rb || dsba == NULL || dsbb == NULL) {
        result = FALSE;
        goto exit;
    }

    if (dsba != dsbac || dsbb != dsbbc) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(dsna);
    RELEASE(dsnb);
    RELEASE(dsba);
    RELEASE(dsbb);
    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}

static BOOL TestDirectSoundCaptureBufferStartCapture(LPDIRECTSOUNDCAPTUREBUFFER a, LPDIRECTSOUNDCAPTUREBUFFER b, DWORD dwSeconds, DWORD dwFlags) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    // GetCaps
    if (FAILED(CompareDirectSoundCaptureBufferCaps(a, b))) {
        return FALSE;
    }

    DWORD cpa = 0, cpb = 0, cwa = 0, cwb = 0;

    HRESULT ra = IDirectSoundCaptureBuffer_GetCurrentPosition(a, &cpa, &cwa);
    HRESULT rb = IDirectSoundCaptureBuffer_GetCurrentPosition(b, &cpb, &cwb);

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

    // Start A

    if (SUCCEEDED(ra = IDirectSoundCaptureBuffer_Start(a, dwFlags))) {
        Sleep(dwSeconds * 1000);
        IDirectSoundCaptureBuffer_Stop(a);
    }

    // Start B

    if (SUCCEEDED(rb = IDirectSoundCaptureBuffer_Start(b, dwFlags))) {
        Sleep(dwSeconds * 1000);
        IDirectSoundCaptureBuffer_Stop(b);
    }

    if (ra != rb) {
        return FALSE;
    }

    ra = IDirectSoundCaptureBuffer_GetCurrentPosition(a, &cpa, &cwa);
    rb = IDirectSoundCaptureBuffer_GetCurrentPosition(b, &cpb, &cwb);

    if (ra != rb) {
        return FALSE;
    }

    if (cpa != cpb || cwa != cwb) {
        IDirectSoundCaptureBuffer_Stop(a);
        IDirectSoundCaptureBuffer_Stop(b);
    }

    return TRUE;
}

static BOOL TestDirectSoundCaptureBufferStartNotify(LPDIRECTSOUNDCAPTURECREATE a, LPDIRECTSOUNDCAPTURECREATE b, DWORD dwFlags) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;
    LPDIRECTSOUNDCAPTUREBUFFER dsba = NULL, dsbb = NULL;

    const DWORD length = 144000;

    WAVEFORMATEX format;
    InitializeWaveFormat(&format, 2, 48000, 8);

    DSCBUFFERDESC desc;
    InitializeDirectSoundCaptureBufferDesc(&desc, 0, length, &format);

    WAVEFORMATEX fa, fb;
    ZeroMemory(&fa, sizeof(WAVEFORMATEX));
    ZeroMemory(&fb, sizeof(WAVEFORMATEX));

    DWORD fas = 0, fbs = 0;

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

    ra = IDirectSoundCapture_CreateCaptureBuffer(dsa, &desc, &dsba, NULL);
    rb = IDirectSoundCapture_CreateCaptureBuffer(dsb, &desc, &dsbb, NULL);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (dsba == NULL || dsbb == NULL) {
        result = FALSE;
        goto exit;
    }

    // GetCaps
    if (FAILED(CompareDirectSoundCaptureBufferCaps(dsba, dsbb))) {
        result = FALSE;
        goto exit;
    }

    // GetFormat

    ra = IDirectSoundCaptureBuffer_GetFormat(dsba, &fa, sizeof(WAVEFORMATEX), &fas);
    rb = IDirectSoundCaptureBuffer_GetFormat(dsbb, &fb, sizeof(WAVEFORMATEX), &fbs);

    if (ra != rb || fas != fbs) {
        result = FALSE;
        goto exit;
    }

    if (memcmp(&fa, &fb, sizeof(WAVEFORMATEX)) != 0) {
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

    HANDLE ha = CreateThread(NULL, 0, NotifyThread, &ctxa, 0, NULL);
    HANDLE hb = CreateThread(NULL, 0, NotifyThread, &ctxb, 0, NULL);

    if (!TestDirectSoundCaptureBufferStartCapture(dsba, dsbb, 4, dwFlags)) {
        result = FALSE;
        goto exit;
    }

    ctxa.Run = FALSE, ctxb.Run = FALSE;

    CloseHandle(ha);
    CloseHandle(hb);

    if (memcmp(signala, signalb, TEST_EVENT_COUNT * sizeof(BOOL)) != 0) {
        // TODO better validation. The number of events doesn't match!

        goto exit;
    }

exit:

    for (int i = 0; i < TEST_EVENT_COUNT; i++) {
        CloseHandle(eventsa[i]);
        CloseHandle(eventsb[i]);
    }

    RELEASE(dsba);
    RELEASE(dsbb);
    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}

BOOL TestDirectSoundCaptureBufferNotify(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCAPTURECREATE dsca = GetDirectSoundCaptureCreate(a);
    LPDIRECTSOUNDCAPTURECREATE dscb = GetDirectSoundCaptureCreate(b);

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    if (!TestDirectSoundCaptureBufferReleaseNotify(dsca, dscb)) {
        return FALSE;
    }

    if (!TestDirectSoundCaptureBufferStartNotify(dsca, dscb, 0)) {
        return FALSE;
    }

    if (!TestDirectSoundCaptureBufferStartNotify(dsca, dscb, DSCBSTART_LOOPING)) {
        return FALSE;
    }

    return TRUE;
}
