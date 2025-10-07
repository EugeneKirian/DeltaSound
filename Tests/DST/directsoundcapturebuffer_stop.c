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

#pragma once

#include "directsoundcapturebuffer.h"

static BOOL TestDirectSoundCaptureBufferStopMethod(LPDIRECTSOUNDCAPTURECREATE a, LPDIRECTSOUNDCAPTURECREATE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    BOOL result = TRUE;
    LPDIRECTSOUNDCAPTURE dsa = NULL, dsb = NULL;
    LPDIRECTSOUNDCAPTUREBUFFER dsba = NULL, dsbb = NULL;

    WAVEFORMATEX format;
    InitializeWaveFormat(&format, 2, 22050, 8);

    DSCBUFFERDESC desc;
    InitializeDirectSoundCaptureBufferDesc(&desc, 0, 132300, &format);

    WAVEFORMATEX fa, fb;
    ZeroMemory(&fa, sizeof(WAVEFORMATEX));
    ZeroMemory(&fb, sizeof(WAVEFORMATEX));

    DWORD fas = 0, fbs = 0;

    DSCBCAPS capsa;
    ZeroMemory(&capsa, sizeof(DSCBCAPS));
    capsa.dwSize = sizeof(DSCBCAPS);

    DSCBCAPS capsb;
    ZeroMemory(&capsb, sizeof(DSCBCAPS));
    capsb.dwSize = sizeof(DSCBCAPS);

    DWORD cpa = 0, cpb = 0, cwa = 0, cwb = 0;

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

    // GetCurrentPosition

    ra = IDirectSoundCaptureBuffer_GetCurrentPosition(dsba, &cpa, &cwa);
    rb = IDirectSoundCaptureBuffer_GetCurrentPosition(dsbb, &cpb, &cwb);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    // Stop 

    ra = IDirectSoundCaptureBuffer_Stop(dsba);
    rb = IDirectSoundCaptureBuffer_Stop(dsbb);

    if (ra != rb) {
        result = FALSE;
        goto exit;
    }

    // GetCurrentPosition

    ra = IDirectSoundBuffer_GetCurrentPosition(dsba, &cpa, &cwa);
    rb = IDirectSoundBuffer_GetCurrentPosition(dsbb, &cpb, &cwb);

    if (ra != rb || cpa != cpb || cwa != cwb) {
        result = FALSE;
        goto exit;
    }

exit:

    RELEASE(dsba);
    RELEASE(dsbb);
    RELEASE(dsa);
    RELEASE(dsb);

    return result;
}

BOOL TestDirectSoundCaptureBufferStop(HMODULE a, HMODULE b) {
    if (a == NULL || b == NULL) {
        return FALSE;
    }

    LPDIRECTSOUNDCAPTURECREATE dsca = GetDirectSoundCaptureCreate(a);
    LPDIRECTSOUNDCAPTURECREATE dscb = GetDirectSoundCaptureCreate(b);

    if (dsca == NULL || dscb == NULL) {
        return FALSE;
    }

    if (!TestDirectSoundCaptureBufferStopMethod(dsca, dscb)) {
        return FALSE;
    }

    return TRUE;
}
