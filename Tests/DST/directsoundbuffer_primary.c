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

#include "directsoundbuffer_primary.h"
#include "wnd.h"

#include <dsound.h>

#define WINDOW_NAME "DirectSound Primary Buffer"

typedef HRESULT(WINAPI* LPDIRECTSOUNDCREATE)(LPCGUID, LPDIRECTSOUND*, LPUNKNOWN);

static BOOL TestDirectSoundBufferGetProperties(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    // GetCaps
    {
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
    }

    // GetCurrentPosition
    {
        DWORD cpca = 0, cwca = 0, cpcb = 0, cwcb = 0;

        HRESULT ra = IDirectSoundBuffer_GetCurrentPosition(a, NULL, NULL);
        HRESULT rb = IDirectSoundBuffer_GetCurrentPosition(b, NULL, NULL);

        if (ra != rb || cpca != cpcb || cwca != cwcb) {
            return FALSE;
        }

        ra = IDirectSoundBuffer_GetCurrentPosition(a, &cpca, &cwca);
        rb = IDirectSoundBuffer_GetCurrentPosition(b, &cpcb, &cwcb);

        if (ra != rb || cpca != cpcb || cwca != cwcb) {
            return FALSE;
        }
    }

    // GetFormat
    {
        const size_t size = 2 * sizeof(WAVEFORMATEX);
        LPWAVEFORMATEX wf1 = malloc(size);

        if (wf1 == NULL) {
            return FALSE;
        }

        ZeroMemory(wf1, size);

        LPWAVEFORMATEX wf2 = malloc(size);

        if (wf2 == NULL) {
            return FALSE;
        }

        ZeroMemory(wf2, size);

        HRESULT ra = IDirectSoundBuffer_GetFormat(a, NULL, 17, NULL);
        HRESULT rb = IDirectSoundBuffer_GetFormat(b, NULL, 17, NULL);

        if (ra != rb) {
            return FALSE;
        }

        DWORD sa = 0, sb = 0;

        ra = IDirectSoundBuffer_GetFormat(a, NULL, 17, &sa);
        rb = IDirectSoundBuffer_GetFormat(b, NULL, 17, &sb);

        if (ra != rb && sa != sb) {
            return FALSE;
        }

        ra = IDirectSoundBuffer_GetFormat(a, wf1, sizeof(WAVEFORMATEX), &sa);
        rb = IDirectSoundBuffer_GetFormat(b, wf2, sizeof(WAVEFORMATEX), &sb);

        if (ra != rb && sa != sb) {
            return FALSE;
        }

        if (memcmp(wf1, wf2, sa) != 0) {
            return FALSE;
        }
        
        sa = 0, sb = 0;

        ra = IDirectSoundBuffer_GetFormat(a, NULL, size, &sa);
        rb = IDirectSoundBuffer_GetFormat(b, NULL, size, &sb);

        if (ra != rb && sa != sb) {
            return FALSE;
        }

        if (memcmp(wf1, wf2, sa) != 0) {
            return FALSE;
        }

        free(wf1);
        free(wf2);
    }

    // GetVolume
    {
        LONG va = 0, vb = 0;

        HRESULT ra = IDirectSoundBuffer_GetVolume(a, NULL);
        HRESULT rb = IDirectSoundBuffer_GetVolume(b, NULL);

        if (ra != rb || va != vb) {
            return FALSE;
        }

        ra = IDirectSoundBuffer_GetVolume(a, &va);
        rb = IDirectSoundBuffer_GetVolume(b, &vb);

        if (ra != rb || va != vb) {
            return FALSE;
        }
    }

    // GetPan
    {
        LONG pa = 0, pb = 0;

        HRESULT ra = IDirectSoundBuffer_GetPan(a, NULL);
        HRESULT rb = IDirectSoundBuffer_GetPan(b, NULL);

        if (ra != rb || pa != pb) {
            return FALSE;
        }

        ra = IDirectSoundBuffer_GetPan(a, &pa);
        rb = IDirectSoundBuffer_GetPan(b, &pb);

        if (ra != rb || pa != pb) {
            return FALSE;
        }
    }

    // GetFrequency
    {
        DWORD fa = 0, fb = 0;

        HRESULT ra = IDirectSoundBuffer_GetFrequency(a, NULL);
        HRESULT rb = IDirectSoundBuffer_GetFrequency(b, NULL);

        if (ra != rb || fa != fb) {
            return FALSE;
        }

        ra = IDirectSoundBuffer_GetFrequency(a, &fa);
        rb = IDirectSoundBuffer_GetFrequency(b, &fb);

        if (ra != rb || fa != fb) {
            return FALSE;
        }
    }

    // GetStatus
    {
        DWORD sa = 0, sb = 0;

        HRESULT ra = IDirectSoundBuffer_GetStatus(a, NULL);
        HRESULT rb = IDirectSoundBuffer_GetStatus(b, NULL);

        if (ra != rb || sa != sb) {
            return FALSE;
        }

        ra = IDirectSoundBuffer_GetStatus(a, &sa);
        rb = IDirectSoundBuffer_GetStatus(b, &sb);

        if (ra != rb || sa != sb) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundBufferPrimaryGetDetails(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb, DWORD flags, DWORD level) {
    if (a == NULL || wa == NULL || b == NULL || wb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    LPDIRECTSOUNDBUFFER dsba = NULL;
    LPDIRECTSOUNDBUFFER dsbb = NULL;

    DSBUFFERDESC desca;
    ZeroMemory(&desca, sizeof(DSBUFFERDESC));

    desca.dwSize = sizeof(DSBUFFERDESC);
    desca.dwFlags = flags;

    DSBUFFERDESC descb;
    ZeroMemory(&descb, sizeof(DSBUFFERDESC));

    descb.dwSize = sizeof(DSBUFFERDESC);
    descb.dwFlags = flags;

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

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (dsba == NULL && dsbb == NULL) {
        return TRUE;
    }

    if (!TestDirectSoundBufferGetProperties(dsba, dsbb)) {
        result = FALSE;
        goto exit;
    }

exit:

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

static BOOL TestDirectSoundBufferSetProperties(LPDIRECTSOUNDBUFFER a, LPDIRECTSOUNDBUFFER b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    DWORD bsa = 0, bsb = 0;

    // SetCurrentPosition
    {
        HRESULT ra = IDirectSoundBuffer_SetCurrentPosition(a, 0);
        HRESULT rb = IDirectSoundBuffer_SetCurrentPosition(b, 0);

        if (ra != rb && ra != DSERR_INVALIDCALL) {
            return FALSE;
        }
    }

    // SetFormat
    {
        const size_t size = 2 * sizeof(WAVEFORMATEX);
        LPWAVEFORMATEX wf1 = malloc(size);

        if (wf1 == NULL) {
            return FALSE;
        }

        ZeroMemory(wf1, size);

        LPWAVEFORMATEX wf2 = malloc(size);

        if (wf2 == NULL) {
            return FALSE;
        }

        ZeroMemory(wf2, size);

        HRESULT ra = IDirectSoundBuffer_GetFormat(a, wf1, sizeof(WAVEFORMATEX), NULL);
        HRESULT rb = IDirectSoundBuffer_GetFormat(b, wf2, sizeof(WAVEFORMATEX), NULL);

        if (ra != rb || memcmp(wf1, wf2, sizeof(WAVEFORMATEX)) != 0) {
            return FALSE;
        }

        LPWAVEFORMATEX wf3 = malloc(size);

        if (wf3 == NULL) {
            return FALSE;
        }

        ZeroMemory(wf3, size);

        LPWAVEFORMATEX wf4 = malloc(size);

        if (wf4 == NULL) {
            return FALSE;
        }

        ZeroMemory(wf4, size);

        ra = IDirectSoundBuffer_SetFormat(a, NULL);
        rb = IDirectSoundBuffer_SetFormat(b, NULL);

        if (ra != rb) {
            return FALSE;
        }

        /* ------------------------------------------- */

        ra = IDirectSoundBuffer_SetFormat(a, wf3);
        rb = IDirectSoundBuffer_SetFormat(b, wf4);

        if (ra != rb) {
            return FALSE;
        }

        ZeroMemory(wf1, sizeof(WAVEFORMATEX));
        ZeroMemory(wf2, sizeof(WAVEFORMATEX));

        ra = IDirectSoundBuffer_GetFormat(a, wf1, sizeof(WAVEFORMATEX), NULL);
        rb = IDirectSoundBuffer_GetFormat(b, wf2, sizeof(WAVEFORMATEX), NULL);

        if (ra != rb || memcmp(wf1, wf2, sizeof(WAVEFORMATEX)) != 0) {
            return FALSE;
        }

        /* ------------------------------------------- */

        wf3->wFormatTag = WAVE_FORMAT_2S16;
        wf4->wFormatTag = WAVE_FORMAT_2S16;

        ra = IDirectSoundBuffer_SetFormat(a, wf3);
        rb = IDirectSoundBuffer_SetFormat(b, wf4);

        if (ra != rb) {
            return FALSE;
        }

        ZeroMemory(wf1, sizeof(WAVEFORMATEX));
        ZeroMemory(wf2, sizeof(WAVEFORMATEX));

        ra = IDirectSoundBuffer_GetFormat(a, wf1, sizeof(WAVEFORMATEX), NULL);
        rb = IDirectSoundBuffer_GetFormat(b, wf2, sizeof(WAVEFORMATEX), NULL);

        if (ra != rb || memcmp(wf1, wf2, sizeof(WAVEFORMATEX)) != 0) {
            return FALSE;
        }

        /* ------------------------------------------- */

        wf3->wFormatTag = WAVE_FORMAT_PCM;
        wf3->nChannels = 16;

        wf4->wFormatTag = WAVE_FORMAT_PCM;
        wf4->nChannels = 16;

        ra = IDirectSoundBuffer_SetFormat(a, wf3);
        rb = IDirectSoundBuffer_SetFormat(b, wf4);

        if (ra != rb) {
            return FALSE;
        }

        ZeroMemory(wf1, sizeof(WAVEFORMATEX));
        ZeroMemory(wf2, sizeof(WAVEFORMATEX));

        ra = IDirectSoundBuffer_GetFormat(a, wf1, sizeof(WAVEFORMATEX), NULL);
        rb = IDirectSoundBuffer_GetFormat(b, wf2, sizeof(WAVEFORMATEX), NULL);

        if (ra != rb || memcmp(wf1, wf2, sizeof(WAVEFORMATEX)) != 0) {
            return FALSE;
        }

        /* ------------------------------------------- */

        wf3->wFormatTag = WAVE_FORMAT_PCM;
        wf3->cbSize = -1;

        wf4->wFormatTag = WAVE_FORMAT_PCM;
        wf4->cbSize = -1;

        ra = IDirectSoundBuffer_SetFormat(a, wf3);
        rb = IDirectSoundBuffer_SetFormat(b, wf4);

        if (ra != rb) {
            return FALSE;
        }

        ZeroMemory(wf1, sizeof(WAVEFORMATEX));
        ZeroMemory(wf2, sizeof(WAVEFORMATEX));

        ra = IDirectSoundBuffer_GetFormat(a, wf1, sizeof(WAVEFORMATEX), NULL);
        rb = IDirectSoundBuffer_GetFormat(b, wf2, sizeof(WAVEFORMATEX), NULL);

        if (ra != rb || memcmp(wf1, wf2, sizeof(WAVEFORMATEX)) != 0) {
            return FALSE;
        }

        /* ------------------------------------------- */

        wf3->wFormatTag = WAVE_FORMAT_PCM;
        wf3->nAvgBytesPerSec = 44100;
        wf3->cbSize = -1;

        wf4->wFormatTag = WAVE_FORMAT_PCM;
        wf4->nAvgBytesPerSec = 44100;
        wf4->cbSize = -1;

        ra = IDirectSoundBuffer_SetFormat(a, wf3);
        rb = IDirectSoundBuffer_SetFormat(b, wf4);

        if (ra != rb) {
            return FALSE;
        }

        ZeroMemory(wf1, sizeof(WAVEFORMATEX));
        ZeroMemory(wf2, sizeof(WAVEFORMATEX));

        ra = IDirectSoundBuffer_GetFormat(a, wf1, sizeof(WAVEFORMATEX), NULL);
        rb = IDirectSoundBuffer_GetFormat(b, wf2, sizeof(WAVEFORMATEX), NULL);

        if (ra != rb || memcmp(wf1, wf2, sizeof(WAVEFORMATEX)) != 0) {
            return FALSE;
        }

        /* ------------------------------------------- */

        wf3->wFormatTag = WAVE_FORMAT_PCM;
        wf3->nChannels = 2;
        wf3->nSamplesPerSec = 22050;
        wf3->nAvgBytesPerSec = 44100 + 1;
        wf3->nBlockAlign = 2;
        wf3->wBitsPerSample = 8;
        wf3->cbSize = -1;

        wf4->wFormatTag = WAVE_FORMAT_PCM;
        wf4->nChannels = 2;
        wf4->nSamplesPerSec = 22050;
        wf4->nAvgBytesPerSec = 44100 + 1;
        wf4->nBlockAlign = 2;
        wf4->wBitsPerSample = 8;
        wf4->cbSize = -1;

        ra = IDirectSoundBuffer_SetFormat(a, wf3);
        rb = IDirectSoundBuffer_SetFormat(b, wf4);

        if (ra != rb) {
            return FALSE;
        }

        ZeroMemory(wf1, sizeof(WAVEFORMATEX));
        ZeroMemory(wf2, sizeof(WAVEFORMATEX));

        ra = IDirectSoundBuffer_GetFormat(a, wf1, sizeof(WAVEFORMATEX), NULL);
        rb = IDirectSoundBuffer_GetFormat(b, wf2, sizeof(WAVEFORMATEX), NULL);

        if (ra != rb || memcmp(wf1, wf2, sizeof(WAVEFORMATEX)) != 0) {
            return FALSE;
        }

        /* ------------------------------------------- */

        free(wf1);
        free(wf2);
        free(wf3);
        free(wf4);
    }

    // SetVolume
    {
        LONG va1 = 0, vb1 = 0, va2 = 0, vb2 = 0;

        HRESULT ra = IDirectSoundBuffer_GetVolume(a, &va1);
        HRESULT rb = IDirectSoundBuffer_GetVolume(b, &vb1);

        if (ra != rb || va1 != vb1) {
            return FALSE;
        }

        ra = IDirectSoundBuffer_SetVolume(a, DSBVOLUME_MIN - 1);
        rb = IDirectSoundBuffer_SetVolume(b, DSBVOLUME_MIN - 1);

        if (ra != rb) {
            return FALSE;
        }

        ra = IDirectSoundBuffer_GetVolume(a, &va1);
        rb = IDirectSoundBuffer_GetVolume(b, &vb1);

        if (ra != rb || va1 != vb1) {
            return FALSE;
        }

        // TODO more tests
    }

    // GetPan
    {
        LONG pa = 0, pb = 0;

        HRESULT ra = IDirectSoundBuffer_GetPan(a, NULL);
        HRESULT rb = IDirectSoundBuffer_GetPan(b, NULL);

        if (ra != rb || pa != pb) {
            return FALSE;
        }

        ra = IDirectSoundBuffer_GetPan(a, &pa);
        rb = IDirectSoundBuffer_GetPan(b, &pb);

        if (ra != rb || pa != pb) {
            return FALSE;
        }
    }

    // GetFrequency
    {
        DWORD fa = 0, fb = 0;

        HRESULT ra = IDirectSoundBuffer_GetFrequency(a, NULL);
        HRESULT rb = IDirectSoundBuffer_GetFrequency(b, NULL);

        if (ra != rb || fa != fb) {
            return FALSE;
        }

        ra = IDirectSoundBuffer_GetFrequency(a, &fa);
        rb = IDirectSoundBuffer_GetFrequency(b, &fb);

        if (ra != rb || fa != fb) {
            return FALSE;
        }
    }

    // GetStatus
    {
        DWORD sa = 0, sb = 0;

        HRESULT ra = IDirectSoundBuffer_GetStatus(a, NULL);
        HRESULT rb = IDirectSoundBuffer_GetStatus(b, NULL);

        if (ra != rb || sa != sb) {
            return FALSE;
        }

        ra = IDirectSoundBuffer_GetStatus(a, &sa);
        rb = IDirectSoundBuffer_GetStatus(b, &sb);

        if (ra != rb || sa != sb) {
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL TestDirectSoundBufferPrimarySetDetails(
    LPDIRECTSOUNDCREATE a, HWND wa, LPDIRECTSOUNDCREATE b, HWND wb, DWORD flags, DWORD level) {
    if (a == NULL || wa == NULL || b == NULL || wb == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;

    LPDIRECTSOUND dsa = NULL;
    LPDIRECTSOUND dsb = NULL;

    LPDIRECTSOUNDBUFFER dsba = NULL;
    LPDIRECTSOUNDBUFFER dsbb = NULL;

    DSBUFFERDESC desca;
    ZeroMemory(&desca, sizeof(DSBUFFERDESC));

    desca.dwSize = sizeof(DSBUFFERDESC);
    desca.dwFlags = flags;

    DSBUFFERDESC descb;
    ZeroMemory(&descb, sizeof(DSBUFFERDESC));

    descb.dwSize = sizeof(DSBUFFERDESC);
    descb.dwFlags = flags;

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

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    if (dsba == NULL && dsbb == NULL) {
        return TRUE;
    }

    if (!TestDirectSoundBufferSetProperties(dsba, dsbb)) {
        result = FALSE;
        goto exit;
    }

exit:

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

#define COOPERATIVE_LEVEL_COUNT 4

static const DWORD CooperativeLevels[COOPERATIVE_LEVEL_COUNT] = {
    DSSCL_NORMAL, DSSCL_PRIORITY, DSSCL_EXCLUSIVE, DSSCL_WRITEPRIMARY
};

#define BUFFER_FLAG_COUNT       7

static const DWORD BufferFlags[BUFFER_FLAG_COUNT] =
{
    DSBCAPS_PRIMARYBUFFER,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRL3D,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_GETCURRENTPOSITION2,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME,
    DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2
};

BOOL TestDirectSoundBufferPrimary(HMODULE a, HMODULE b) {
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

    //for (int i = 0; i < COOPERATIVE_LEVEL_COUNT; i++) {
    //    for (int k = 0; k < BUFFER_FLAG_COUNT; k++) {
    //        if (!TestDirectSoundBufferPrimaryGetDetails(dsca, wa, dscb, wb, BufferFlags[k], CooperativeLevels[i])) {
    //            result = FALSE;
    //            goto exit;
    //        }
    //    }
    //}

    for (int i = 0; i < COOPERATIVE_LEVEL_COUNT; i++) {
        for (int k = 0; k < BUFFER_FLAG_COUNT; k++) {
            if (!TestDirectSoundBufferPrimarySetDetails(dsca, wa, dscb, wb, BufferFlags[k], CooperativeLevels[i])) {
                result = FALSE;
                goto exit;
            }
        }
    }

    // TODO more tests


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
